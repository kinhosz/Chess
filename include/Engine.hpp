#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

#include <Game.hpp>

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

typedef std::pair<int, int> i2;
typedef std::pair<i2, i2> i4;
typedef std::pair<i4, int> i5;

double INF = 1e8;

int cmp(double a, double b) {
  double eps = 0.0001;

  if(std::abs(a - b) < eps) return 0;
  if(a < b) return -1;
  return 1;
}

bool max_cmp(std::pair<double, int> a, std::pair<double, int> b) {
  return cmp(a.first, b.first) == 1;
}

bool min_cmp(std::pair<double, int> a, std::pair<double, int> b) {
  return cmp(a.first, b.first) == -1;
}

class EngineNode {
private:
  double score;
  int level;
  int next_line;
  std::vector<std::unique_ptr<EngineNode>> lines;
  std::vector<std::pair<double, int>> sorted_ptr;

  void createNextLines(Game& game) {
    const auto& moves = game.getAllMoves();

    bool isWhiteTurn = game.isWhiteTurn();

    for(int i=0;i<moves.size();i++) {
      const auto &move = moves[i];
      if(game.isPawnPromotion(move.first, move.second)) {
        for(int i=0;i<4;i++) {
          lines.push_back(std::make_unique<EngineNode>(std::make_pair(move, i), level));
        }
      } else {
        lines.push_back(std::make_unique<EngineNode>(std::make_pair(move, -1), level + 1));
      }

      sorted_ptr.push_back({0.0, i});
    }
  }

  bool isLinesMissing(const Game& game) const {
    if(game.isDraw() || game.isCheckMate()) return false;
    return lines.size() == 0;
  }

public:
  i5 move;

  EngineNode(i5 move, int level) {
    this->move = move;
    this->level = level;
    score = 0.0;
    next_line = -1;
  }

  void setScore(double sc) {
    score = sc;
  }

  double getScore() const {
    return score;
  }

  double explore(Game& game, int deep, double alpha, double beta, int &cnt) {
    cnt++;
    score = game.getScore();

    if(deep <= 0) return score;
    if(game.isDraw() || game.isCheckMate()) return score;
    if(isLinesMissing(game)) createNextLines(game);

    double first_assign = true;
    bool whiteTurn = game.isWhiteTurn();

    score = (game.isWhiteTurn() ? -INF: INF);
    int break_i = sorted_ptr.size();

    for(int i=0;i<sorted_ptr.size();i++) {
      int ptr = sorted_ptr[i].second;
      const auto &line = lines[ptr];
  
      game.doAction(line->move.first.first, line->move.first.second, line->move.second);

      double sc = line->explore(game, deep-1, alpha, beta, cnt);
      sorted_ptr[i].first = sc;

      game.undoAction(); // Rollback

      if(first_assign) {
        score = sc;
        first_assign = false;
      } else if(whiteTurn) {
        score = std::max(score, sc);
      } else {
        score = std::min(score, sc);
      }

      // Alpha-beta prunning (cutoff)
      if(whiteTurn) {
        if(cmp(score, beta) != -1) {
          score = 1000.0; // To avoid use this branch as we dont calculate it until the end
          break_i = i;
          break;
        }
        alpha = std::max(alpha, score);
      } else {
        if(cmp(score, alpha) != 1) {
          score = -1000.0;
          break_i = i;
          break;
        }
        beta = std::min(beta, score);
      }
    }

    for(int i=break_i;i<sorted_ptr.size();i++) {
      sorted_ptr[i].first = score;
    }

    // For some reason, sort after is better the before?
    if(whiteTurn) std::sort(sorted_ptr.begin(), sorted_ptr.end(), max_cmp);
    else std::sort(sorted_ptr.begin(), sorted_ptr.end(), min_cmp);

    return score;
  }

  i5 getNextMove(Game &game, int deep, int &cnt) {
    if(next_line != -1){
      i5 m = lines[next_line]->move;
      game.doAction(m.first.first, m.first.second, m.second);
      i5 ret = lines[next_line]->getNextMove(game, deep, cnt);
      game.undoAction();

      return ret;
    }

    double alpha = -INF;
    double beta = INF;
    score = explore(game, deep, alpha, beta, cnt);
    std::cerr << cnt << " nodes generated\n";
    std::vector<int> goodMoves;

    int score_int = score * 10;

    for(int i=0;i<sorted_ptr.size();i++) {
      int curr_score_int = sorted_ptr[i].first * 10;
      if(curr_score_int == score_int) goodMoves.push_back(sorted_ptr[i].second);
    }
    std::cerr << "good moves: " << goodMoves.size() << "\n";

    int pt = std::uniform_int_distribution<int>(0, (int)goodMoves.size() - 1)(rng);
    int choose = goodMoves[pt];

    std::cerr << "future score: " << lines[choose]->score << "\n";

    return lines[choose]->move;
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

    assert(next_line != -1);
  }
};

class Engine {
private:
  std::unique_ptr<EngineNode> root;
  std::vector<i5> cacheMoves;
  Game game;

  void cleanCache() {
    for(int i=0;i<cacheMoves.size();i++) {
      root->moveDone(game, cacheMoves[i]);
    }
    cacheMoves.clear();
  }

public:

  Engine() {
    i5 move = {{{-1, -1}, {-1, -1}}, -1};
    root = std::make_unique<EngineNode>(move, 0);
  }

  i5 getNextMove(int deep_size) {
    cleanCache();
    int cnt = 0;
    auto ret = root->getNextMove(game, deep_size, cnt);
    return ret;
  }

  void moveDone(i5 move) {
    cacheMoves.push_back(move);
  }

  void performance() {
    game.performance();
  }
};

#endif
