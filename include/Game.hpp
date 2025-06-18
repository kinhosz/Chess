#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <vector>
#include <assert.h>

typedef std::pair<int, int> pii;

class Game {
private:
  std::vector<pii> markedCells;
  bool whiteTurn;
  pii enPassant;
  bool isLeftWhiteRookMoved;
  bool isRightWhiteRookMoved;
  bool isWhiteKingMoved;
  bool isLeftBlackRookMoved;
  bool isRightBlackRookMoved;
  bool isBlackKingMoved;
  pii activeCell;
  int state;

  std::vector<std::vector<std::string>> board;
  std::vector<std::pair<pii, pii>> nextMoves;

  void buildBoard();
  std::string getPositionInfo(int x, int y);
  bool isOnCheck();
  bool isValidMove(pii current_pos, pii new_pos);
  void genNextMoves();
  void resetEnPassant();
  void doAction(pii current_pos, pii new_pos);
  pii getKingPos(bool white) const;

public:
  Game();

  std::vector<std::vector<std::string>> getBoard() const;
  void handleClickOnCell(pii cell);
  std::vector<std::pair<pii, int>> getSpecialCells() const;
  bool isDraw() const;
  bool isCheckMate() const;
};

#endif
