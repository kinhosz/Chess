#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <map>
#include <vector>
#include <assert.h>

typedef std::pair<int, int> pii;

struct GameState {
  pii enPassant;
  int castlingPreserved;
  std::string gameStatus;

  bool isCastlingPreserved(int id) const {
    // 0: o-o-o white, 1: o-o white, 2: o-o-o black, 3: o-o black
    assert(id >= 0 && id <= 3);
    return (castlingPreserved & (1<<id)) == 0;
  }

  void touch(int id) {
    assert(id >= 0 && id <= 3);
    castlingPreserved |= (1<<id);
  }
};

class Game {
private:
  std::vector<GameState> gameState;
  std::vector<std::vector<std::string>> board;
  std::vector<std::pair<pii, pii>> nextMoves;
  std::map<std::string, int> hashedBoardCounter;
  std::vector<std::vector<std::pair<pii, std::string>>> moves;

  GameState getState() const;
  void addState(GameState gs);

  void buildBoard();
  std::string getBoardHash() const;
  void storeHashedBoard();
  std::string getPositionInfo(int x, int y) const;
  bool isValidMove(pii curr_pos, pii new_pos);
  bool isOnCheck() const;
  void genNextMoves(const GameState gs);
  pii getKingPos(bool white) const;
  bool drawConditions() const;
  double executeMove(std::vector<std::pair<pii, std::string>> &move);
  double evaluatePiece(std::string piece) const;

public:
  Game();

  std::vector<std::vector<std::string>> getBoard(int move_id=-1) const;
  void undoAction();
  double doAction(pii current_pos, pii new_pos, int choose=-1);
  std::vector<std::pair<pii, int>> getSpecialCells(pii cell) const;
  bool isDraw() const;
  bool isCheckMate() const;
  bool isWhiteTurn() const;
  bool hasMoveFor(pii pos) const;
  bool isPawnPromotion(pii curr_pos, pii new_pos) const;
  bool isAvailable(pii curr_pos, pii new_pos) const;
  int getTotalMoves() const;
  std::vector<std::pair<pii, pii>> getAllMoves() const;
};

#endif
