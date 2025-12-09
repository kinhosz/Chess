#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <map>
#include <vector>
#include <assert.h>
#include <math.h>
#include <Define.hpp>

typedef std::pair<int, int> pii;

struct GameState {
  pii enPassant;
  int castlingPreserved;
  int gameStatus;
  double piecesScoring;
  int moves_white;
  int moves_black;
  bool repetition;
  int castled;

  std::vector<int> pieces_counter;

  bool isCastlingPreserved(int id) const {
    // 0: o-o-o white, 1: o-o white, 2: o-o-o black, 3: o-o black
    assert(id >= 0 && id <= 3);
    return (castlingPreserved & (1<<id)) == 0;
  }

  void touch(int id) {
    assert(id >= 0 && id <= 3);
    castlingPreserved |= (1<<id);
  }

  void doCastling(bool side) {
    castled |= (1<<side);
  }

  double scoringHeuristic() const {
    double sc = (moves_white - moves_black) / sqrt(moves_white + moves_black + 1.0); // moves
    sc /= 10.0;

    double castling_bonus = 1.5;

    if(castled&1) sc += castling_bonus; // bonus - white castling
    else if((castlingPreserved&3) == 0) sc += castling_bonus / 2.0;

    if(castled&2) sc -= castling_bonus; // bonus - black castling
    else if((castlingPreserved&12) == 0) sc -= castling_bonus / 2.0;

    return sc;
  }
};

class Game {
private:
  std::vector<GameState> gameState;
  std::vector<std::vector<int>> board;
  std::vector<std::pair<pii, pii>> nextMoves;
  std::map<std::string, int> hashedBoardCounter;
  std::vector<std::vector<std::pair<pii, int>>> moves;

  // Performance
  std::map<std::string, double> elapsed_sec;
  std::map<std::string, int> called_counter;

  // Bitboard
  uint64_t board_mask;
  std::vector<uint64_t> bishop_mask;
  std::vector<uint64_t> rook_mask;
  std::vector<uint64_t> knight_mask;
  std::vector<uint64_t> king_mask;
  std::vector<uint64_t> pawn_mask;
  int king_pos[2];

  void boardMaskOccupancy();
  void bishopMaskOccupancy();
  void rookMaskOccupancy();
  void knightMaskOccupancy();
  void kingMaskOccupancy();
  void pawnMaskOccupancy();
  void setMaskPosition(int prev_piece, int new_piece, pii position);

  void setBoard(int x, int y, int piece);

  GameState getState() const;
  void addState(GameState gs);

  void buildBoard();
  std::string getBoardHash();
  int storeHashedBoard();
  int getPositionInfo(int x, int y) const;
  bool isValidMove(pii curr_pos, pii new_pos);
  bool isOnCheck();
  void genNextMoves(const GameState &gs);
  pii getKingPos(bool white);
  bool drawConditions(const GameState &gs) const;
  void executeMove(std::vector<std::pair<pii, int>> &move, GameState &gs);
  double evaluatePiece(int piece) const;

public:
  Game();

  std::vector<std::vector<int>> getBoard(int move_id=-1);
  void undoAction();
  void doAction(pii current_pos, pii new_pos, int choose=-1);
  std::vector<std::pair<pii, int>> getSpecialCells(pii cell);
  bool isDraw() const;
  bool isCheckMate() const;
  bool isWhiteTurn() const;
  bool hasMoveFor(pii pos);
  bool isPawnPromotion(pii curr_pos, pii new_pos);
  bool isAvailable(pii curr_pos, pii new_pos);
  int getTotalMoves() const;
  std::vector<std::pair<pii, pii>> getAllMoves();
  double getScore() const;
  double getCellScore(int x, int y) const;

  // Performance
  void performance();
};

#endif
