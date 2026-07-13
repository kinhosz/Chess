#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <map>
#include <vector>
#include <cstdint>
#include <assert.h>
#include <math.h>
#include <Define.hpp>

// Every fractional constant the evaluation ever used (bonus_factor=0.1,
// mobility divisors 15/16/8, castling_bonus=1.5) is folded into this single
// scale, chosen as lcm(50,32,80,10,4) -- the smallest factor that turns every
// one of those fractions into a whole number, so all scoring is exact
// integer arithmetic (no runtime division, no truncation-before-multiply).
constexpr int EVAL_SCALE = 800;

constexpr Score PIECE_VALUE_PAWN   = 1 * EVAL_SCALE;
constexpr Score PIECE_VALUE_KNIGHT = 3 * EVAL_SCALE;
constexpr Score PIECE_VALUE_BISHOP = 3 * EVAL_SCALE;
constexpr Score PIECE_VALUE_ROOK   = 5 * EVAL_SCALE;
constexpr Score PIECE_VALUE_QUEEN  = 9 * EVAL_SCALE;

// Mobility bonus per free square, e.g. bishop: 3 * 0.1 * EVAL_SCALE / 15 = 16
// exactly (same derivation for rook/knight with their own divisor).
constexpr Score BISHOP_MOBILITY_UNIT = 16;
constexpr Score ROOK_MOBILITY_UNIT   = 25;
constexpr Score KNIGHT_MOBILITY_UNIT = 30;
// Pawn-structure defender bonus and per-rank advancement bonus, both were
// `* 0.1` in the old formula: 0.1 * EVAL_SCALE = 80.
constexpr Score PAWN_STRUCTURE_UNIT = 80;

constexpr Score CASTLING_BONUS      = 1200; // 1.5  * EVAL_SCALE
constexpr Score CASTLING_HALF_BONUS = 600;  // 0.75 * EVAL_SCALE

constexpr Score CHECKMATE_SCORE = 1000 * EVAL_SCALE;

struct GameState {
  i2 enPassant;
  int castlingPreserved;
  int gameStatus;
  Score piecesScoring;
  bool repetition;
  int castled;
  bool hasMoves;
  uint64_t zobristHash;

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

  Score scoringHeuristic() const {
    Score sc = 0;

    if(castled&1) sc += CASTLING_BONUS; // bonus - white castling
    else if((castlingPreserved&3) == 0) sc += CASTLING_HALF_BONUS;

    if(castled&2) sc -= CASTLING_BONUS; // bonus - black castling
    else if((castlingPreserved&12) == 0) sc -= CASTLING_HALF_BONUS;

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
  std::map<uint64_t, int> hashedBoardCounter;
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
  int storeHashedBoard(uint64_t hash);
  int getPositionInfo(int x, int y) const;
  bool isValidMove(i2 curr_pos, i2 new_pos);
  bool isOnCheck();
  i2 getKingPos(bool white);
  bool drawConditions(const GameState &gs) const;
  void executeMove(vi3 &move, GameState &gs);
  Score evaluatePiece(int piece) const;
  bool isAttackedBy(i2 pos, int attackerColor) const;
  Score hangingPiecesScore() const;

  vi4 getMovesForPawn(i2 current_pos);
  vi4 getMovesForRook(i2 current_pos);
  vi4 getMovesForKnight(i2 current_pos);
  vi4 getMovesForBishop(i2 current_pos);
  vi4 getMovesForQueen(i2 current_pos);
  vi4 getMovesForKing(i2 current_pos);
  vi4 getMovesFor(i2 pos);
  bool hasAnyMove();

  Score positionalScoring() const;

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
  Score getScore() const;
  Score getCellScore(int x, int y) const;

  // Debugger;
  void debugger();
};

#endif
