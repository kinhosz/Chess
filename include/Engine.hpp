#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <memory>
#include <vector>
#include <chrono>
#include <random>

#include <Game.hpp>

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

typedef std::pair<int, int> i2;
typedef std::pair<i2, i2> i4;
typedef std::pair<i4, int> i5;

class EngineNode {
private:
  int level;
  int next_line;
  std::vector<std::unique_ptr<EngineNode>> lines;
  std::vector<double> scores;

  void createNextLines(const Game& game) {
    const auto& moves = game.getAllMoves();

    for(auto &move: moves) {
      if(game.isPawnPromotion(move.first, move.second)) {
        for(int i=0;i<4;i++) {
          lines.push_back(std::make_unique<EngineNode>(std::make_pair(move, i), level));
        }
      } else {
        lines.push_back(std::make_unique<EngineNode>(std::make_pair(move, -1), level + 1));
      }
    }
  }

  bool isLinesMissing(const Game& game) const {
    if(game.isDraw() || game.isCheckMate()) return false;
    return lines.size() == 0;
  }

public:
  i5 move;
  double score;

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

  void explore(Game& game, int deep) {
    score = 0.0;

    if(deep <= 0) return;
    if(isLinesMissing(game)) createNextLines(game);

    double first_assign = true;
    int whiteTurn = game.isWhiteTurn();
    scores.clear();

    for(const auto &line: lines) {
      double tmp_score = game.doAction(line->move.first.first, line->move.first.second, line->move.second);
      line->explore(game, deep-1);
      tmp_score += (line->score);
      scores.push_back(tmp_score);

      game.undoAction(); // Rollback

      if(first_assign) {
        score = tmp_score;
        first_assign = false;
      } else if(whiteTurn) {
        score = std::max(score, tmp_score);
      } else {
        score = std::min(score, tmp_score);
      }
    }
  }

  void moveDone(Game &game, i5 move) {
    if(isLinesMissing(game)) createNextLines(game);

    if(next_line != -1) {
      game.doAction(lines[next_line]->move.first.first, lines[next_line]->move.first.second, lines[next_line]->move.second);
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

  i5 getNextMove(Game &game) {
    if(isLinesMissing(game)) createNextLines(game);

    if(next_line != -1){
      i5 m = lines[next_line]->move;
      game.doAction(m.first.first, m.first.second, m.second);
      i5 ret = lines[next_line]->getNextMove(game);
      game.undoAction();

      return ret;
    }

    int choose = std::uniform_int_distribution<int>(0, (int)lines.size() - 1)(rng);

    return lines[choose]->move;
  }
};

class Engine {
private:
  std::unique_ptr<EngineNode> root;
  Game game;

public:

  Engine() {
    i5 move = {{{-1, -1}, {-1, -1}}, -1};
    root = std::make_unique<EngineNode>(move, 0);
  }

  i5 getNextMove(int deep_size) {
    return root->getNextMove(game);
  }

  void moveDone(i5 move) {
    root->moveDone(game, move);
  }
};

#endif
