#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <map>
#include <vector>
#include <assert.h>

typedef std::pair<int, int> pii;

class Game {
private:
  bool whiteTurn;
  pii enPassant;
  bool isLeftWhiteRookMoved;
  bool isRightWhiteRookMoved;
  bool isWhiteKingMoved;
  bool isLeftBlackRookMoved;
  bool isRightBlackRookMoved;
  bool isBlackKingMoved;
  int state;

  std::vector<std::vector<std::string>> board;
  std::vector<std::pair<pii, pii>> nextMoves;
  std::map<int, int> hashedBoardCounter;
  std::vector<std::vector<std::pair<pii, std::string>>> backupCells;

  void buildBoard();
  void storeHashedBoard();
  std::string getPositionInfo(int x, int y) const;
  bool isValidMove(pii curr_pos, pii new_pos);
  bool isOnCheck() const;
  void genNextMoves();
  void resetEnPassant();
  pii getKingPos(bool white) const;
  bool drawConditions() const;
  void executeMove(std::vector<std::pair<pii, std::string>> &move);

public:
  Game();

  std::vector<std::vector<std::string>> getBoard(int move_id=-1) const;
  void doAction(pii current_pos, pii new_pos, int choose=-1);
  std::vector<std::pair<pii, int>> getSpecialCells(pii cell) const;
  bool isDraw() const;
  bool isCheckMate() const;
  bool isWhiteTurn() const;
  bool hasMoveFor(pii pos) const;
  bool isPawnPromotion(pii curr_pos, pii new_pos) const;
  bool isAvailable(pii curr_pos, pii new_pos) const;
  int getTotalMoves() const;
};

#endif
