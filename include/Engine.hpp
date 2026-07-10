#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <cstdlib>
#include <memory>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

#include <Game.hpp>
#include <Define.hpp>
#include <Profiler.hpp>

// Randomness (move shuffling in createNextLines + tie-break selection in
// getNextMove) is seeded from CHESS_SEED when set, so a whole game -- shuffle
// order and all -- can be replayed bit-for-bit to reproduce a specific bad
// move: `CHESS_SEED=1 ./selfplay ...`. Defaults to a fixed seed (not the
// clock) so every run is reproducible out of the box; set CHESS_SEED to any
// other integer to get a different (still reproducible) game.
inline unsigned long chessRngSeed() {
  const char *seedEnv = std::getenv("CHESS_SEED");
  if(seedEnv) return std::strtoul(seedEnv, nullptr, 10);
  return 1;
}

// Captured once so it can be logged (e.g. MatchPage prints it at game start)
// -- otherwise there'd be no way to tell, after the fact, which seed a given
// game's log actually used.
unsigned long CHESS_RNG_SEED_USED = chessRngSeed();
std::mt19937 rng(CHESS_RNG_SEED_USED);

double INF = 1e8;

int cmp(double a, double b) {
  double eps = 0.0001;

  if(std::abs(a - b) < eps) return 0;
  if(a < b) return -1;
  return 1;
}

// std::sort requires a strict weak ordering; cmp()'s epsilon tolerance makes
// "equality" non-transitive, so sorting must use plain comparisons here.
bool max_cmp(std::pair<double, int> a, std::pair<double, int> b) {
  return a.first > b.first;
}

bool min_cmp(std::pair<double, int> a, std::pair<double, int> b) {
  return a.first < b.first;
}

// One ranked candidate move from the root of a search, for explaining why a
// move was picked over the alternatives.
struct MoveCandidate {
  i5 move;
  double score;
};

// Optional output of Engine::getNextMove(): the ranked root candidates plus
// the principal variation (the line of best replies the search expects to
// follow from the chosen move), for logging "why" a move was chosen.
struct MoveExplanation {
  std::vector<MoveCandidate> candidates; // best-first
  std::vector<i5> principalVariation;    // starts with the chosen move
};

class EngineNode {
  // Test-only seam: lets tests/engine_test_access.hpp inspect lines/sorted_ptr
  // directly (e.g. to check they stay index-aligned through createNextLines).
  // No production code uses this.
  friend struct TestEngineAccess;

private:
  double score;
  int next_line;
  std::vector<std::unique_ptr<EngineNode>> lines;
  std::vector<std::pair<double, int>> sorted_ptr;

  void createNextLines(Game& game) {
    Profiler::getInstance().start("EngineNode::createNextLines");
    const auto& moves = game.genNextMoves();

    bool isWhiteTurn = game.isWhiteTurn();

    for(int i=0;i<moves.size();i++) {
      const auto &move = moves[i];
      if(game.isPawnPromotion(move.first, move.second)) {
        for(int p=0;p<4;p++) {
          lines.push_back(std::make_unique<EngineNode>(std::make_pair(move, p)));
          sorted_ptr.push_back({0.0, (int)lines.size() - 1});
        }
      } else {
        lines.push_back(std::make_unique<EngineNode>(std::make_pair(move, -1)));
        sorted_ptr.push_back({0.0, (int)lines.size() - 1});
      }
    }

    std::shuffle(sorted_ptr.begin(), sorted_ptr.end(), rng);
    Profiler::getInstance().stop("EngineNode::createNextLines");
  }

  bool isLinesMissing(const Game& game) const {
    if(game.isDraw() || game.isCheckMate()) return false;
    return lines.size() == 0;
  }

public:
  i5 move;

  EngineNode(i5 move) {
    this->move = move;
    score = 0.0;
    next_line = -1;
  }

  void setScore(double sc) {
    score = sc;
  }

  double getScore() const {
    return score;
  }

  // Index into `lines` of whichever child produced this node's most recently
  // returned `score` -- the one child whose value is guaranteed exact for
  // this call, as opposed to the fail-soft *bounds* alpha-beta leaves behind
  // on the other siblings (see the comment above bestChild below).
  int lastBestChild = -1;

  double explore(Game& game, int deep, double alpha, double beta, int &cnt) {
    Profiler::getInstance().start("EngineNode::explore");
    cnt++;
    score = game.getScore();
    lastBestChild = -1; // no children explored yet this call; leaves stay -1

    if(deep <= 0) { Profiler::getInstance().stop("EngineNode::explore"); return score; }
    if(game.isDraw() || game.isCheckMate()) { Profiler::getInstance().stop("EngineNode::explore"); return score; }
    if(isLinesMissing(game)) createNextLines(game);

    bool whiteTurn = game.isWhiteTurn();

    score = (game.isWhiteTurn() ? -INF: INF);
    int break_i = sorted_ptr.size();
    // The other siblings' sorted_ptr[*].first values are alpha-beta *bounds*,
    // not exact scores (each was explored under whatever alpha/beta this
    // loop had tightened to by the time it was reached, so a sibling visited
    // later can return a merely-good-enough cutoff value that happens to
    // numerically match, or even beat, the true best without actually being
    // that good). Only the child that actually set the returned `score` via
    // strict improvement below is guaranteed exact -- track it directly
    // instead of re-scanning sorted_ptr for "ties" against a bound afterward.
    int bestChild = -1;

    for(int i=0;i<sorted_ptr.size();i++) {
      int ptr = sorted_ptr[i].second;
      const auto &line = lines[ptr];

      game.doAction(line->move.first.first, line->move.first.second, line->move.second);

      double sc = line->explore(game, deep-1, alpha, beta, cnt);


      sorted_ptr[i].first = sc;

      game.undoAction(); // Rollback

      bool improved = whiteTurn ? (sc > score) : (sc < score);
      if(improved) { score = sc; bestChild = ptr; }

      // Alpha-beta prunning (cutoff)
      if(whiteTurn) {
        if(cmp(score, beta) != -1) {
          break_i = i;
          break;
        }
        alpha = std::max(alpha, score);
      } else {
        if(cmp(score, alpha) != 1) {
          break_i = i;
          break;
        }
        beta = std::min(beta, score);
      }
    }

    lastBestChild = bestChild;

    // [0, break_i) were fully evaluated and proven worse than the cutoff line;
    // sort just that slice, then rotate it behind the cutoff line and the
    // untouched (still unexplored) lines, preserving their prior ordering.
    if(whiteTurn) std::sort(sorted_ptr.begin(), sorted_ptr.begin() + break_i, max_cmp);
    else std::sort(sorted_ptr.begin(), sorted_ptr.begin() + break_i, min_cmp);

    std::rotate(sorted_ptr.begin(), sorted_ptr.begin() + break_i, sorted_ptr.end());

    Profiler::getInstance().stop("EngineNode::explore");
    return score;
  }

  // Walks down from this node following, at each step, the child that
  // explore() proved is exactly this node's best continuation -- following
  // lastBestChild rather than re-scanning sorted_ptr for a numeric match,
  // since siblings can hold fail-soft bounds that coincidentally equal the
  // best score without actually being that good.
  void collectPV(std::vector<i5> &pv, int maxLen) const {
    const EngineNode *cur = this;
    for(int step=0; step<maxLen && cur->lastBestChild != -1; step++) {
      int bestIdx = cur->lastBestChild;
      pv.push_back(cur->lines[bestIdx]->move);
      cur = cur->lines[bestIdx].get();
    }
  }

  i5 getNextMove(Game &game, int deep, int &cnt, MoveExplanation *explain_out = nullptr) {
    if(next_line != -1){
      i5 m = lines[next_line]->move;
      game.doAction(m.first.first, m.first.second, m.second);
      i5 ret = lines[next_line]->getNextMove(game, deep, cnt, explain_out);
      game.undoAction();

      return ret;
    }

    double alpha = -INF;
    double beta = INF;
    score = explore(game, deep, alpha, beta, cnt);

    // lastBestChild (set by explore() above) is the one child proven to
    // exactly achieve `score` -- unlike its siblings, which may only hold
    // fail-soft bounds that happen to look tied. Use it directly instead of
    // re-scanning sorted_ptr with an epsilon comparison and picking randomly
    // among "ties" that might not really be ties.
    int choose = lastBestChild;
    assert(choose != -1);

    if(explain_out) {
      explain_out->candidates.clear();
      for(auto &sp: sorted_ptr) explain_out->candidates.push_back({lines[sp.second]->move, sp.first});
      bool whiteTurn = game.isWhiteTurn();
      std::sort(explain_out->candidates.begin(), explain_out->candidates.end(),
        [whiteTurn](const MoveCandidate &a, const MoveCandidate &b) {
          return whiteTurn ? a.score > b.score : a.score < b.score;
        });

      explain_out->principalVariation.clear();
      explain_out->principalVariation.push_back(lines[choose]->move);
      lines[choose]->collectPV(explain_out->principalVariation, deep - 1);
    }

    i5 finalMove = lines[choose]->move;
    // Logged directly at the decision point (not by callers) so it's always
    // present regardless of who's driving the engine -- selfplay, the real
    // match UI, or a debugging script -- with no extra flag to remember.
    std::cerr << "[ENGINE][DECISION] " << (game.isWhiteTurn() ? "white" : "black")
               << " chose " << squareName(finalMove.first.first) << squareName(finalMove.first.second)
               << " promo=" << finalMove.second << " score=" << score
               << " depth=" << deep << " nodes=" << cnt << "\n";

    return finalMove;
  }

  void moveDone(Game &game, i5 move) {
    if(isLinesMissing(game)) createNextLines(game);

    if(next_line != -1) {
      i5 m = lines[next_line]->move;
      game.doAction(m.first.first, m.first.second, m.second);
      lines[next_line]->moveDone(game, move);
      game.undoAction();
      return;
    }

    for(int i=0;i<lines.size();i++) {
      if(lines[i]->move == move) {
        next_line = i;
        break;
      }
    }

    // lines[i] for i != next_line will not be used and must be cleaned to save memory
    std::swap(lines[next_line], lines[0]);
    lines.resize(1);
    sorted_ptr.clear();
    next_line = 0;
  }
};

class Engine {
private:
  std::unique_ptr<EngineNode> root;
  Game game;

public:

  Engine() {
    i5 move = {{{-1, -1}, {-1, -1}}, -1};
    root = std::make_unique<EngineNode>(move);
  }

  i5 getNextMove(int deep_size, int *nodes_out = nullptr, MoveExplanation *explain_out = nullptr) {
    int cnt = 0;
    auto ret = root->getNextMove(game, deep_size, cnt, explain_out);
    if(nodes_out) *nodes_out = cnt;
    return ret;
  }

  void moveDone(i5 move) {
    root->moveDone(game, move);
  }
};

#endif
