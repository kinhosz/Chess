#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <map>
#include <vector>
#include <assert.h>
#include <math.h>
#include <Define.hpp>

struct GameState {
  i2 enPassant;
  int castlingPreserved;
  int gameStatus;
  double piecesScoring;
  bool repetition;
  int castled;
  bool hasMoves;

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
    double sc = 0.0;

    double castling_bonus = 1.5;

    if(castled&1) sc += castling_bonus; // bonus - white castling
    else if((castlingPreserved&3) == 0) sc += castling_bonus / 2.0;

    if(castled&2) sc -= castling_bonus; // bonus - black castling
    else if((castlingPreserved&12) == 0) sc -= castling_bonus / 2.0;

    return sc;
  }
};

class Game {
  // Test-only seam: lets tests/position_builder.hpp drop arbitrary positions
  // onto the board (endgames, stalemates) without needing a 20+ move legal
  // sequence to reach them. No production code uses this.
  friend struct TestPositionBuilder;

private:
  std::vector<GameState> gameState;
  std::vector<std::vector<int>> board;
  std::map<std::string, int> hashedBoardCounter;
  std::vector<vi3> moves;

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
  void setMaskPosition(int prev_piece, int new_piece, i2 position);

  void setBoard(int x, int y, int piece);

  GameState getState() const;
  void addState(GameState gs);
  void popState();

  void buildBoard();
  std::string getBoardHash();
  int storeHashedBoard();
  int getPositionInfo(int x, int y) const;
  bool isValidMove(i2 curr_pos, i2 new_pos);
  bool isOnCheck();
  i2 getKingPos(bool white);
  bool drawConditions(const GameState &gs) const;
  void executeMove(vi3 &move, GameState &gs);
  double evaluatePiece(int piece) const;
  bool isAttackedBy(i2 pos, int attackerColor) const;
  double hangingPiecesScore() const;

  vi4 getMovesForPawn(i2 current_pos);
  vi4 getMovesForRook(i2 current_pos);
  vi4 getMovesForKnight(i2 current_pos);
  vi4 getMovesForBishop(i2 current_pos);
  vi4 getMovesForQueen(i2 current_pos);
  vi4 getMovesForKing(i2 current_pos);
  vi4 getMovesFor(i2 pos);
  bool hasAnyMove();

  double positionalScoring() const;

public:
  Game();

  std::vector<std::vector<int>> getBoard(int move_id=-1);
  void undoAction();
  void doAction(i2 current_pos, i2 new_pos, int choose=-1);
  vi3 getSpecialCells(i2 cell);
  bool isDraw() const;
  bool isCheckMate() const;
  bool isWhiteTurn() const;
  bool hasMoveFor(i2 pos);
  bool isPawnPromotion(i2 curr_pos, i2 new_pos);
  bool isAvailable(i2 curr_pos, i2 new_pos);
  int getTotalMoves() const;
  vi4 genNextMoves();
  double getScore() const;
  double getCellScore(int x, int y) const;

  // Debugger;
  void debugger();
};

#endif
