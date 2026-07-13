#ifndef ZOBRIST_HPP
#define ZOBRIST_HPP

#include <cstdint>
#include <random>
#include <vector>
#include <Define.hpp>
#include <Bitboard.hpp>

// Fixed forever, independent of CHESS_SEED (see Engine.hpp) -- CHESS_SEED is
// for game-move reproducibility (shuffle/tie-break) and may change run to
// run; this seed must not, or a previously computed hash value would stop
// meaning the same position.
constexpr uint64_t ZOBRIST_SEED = 0x9E3779B97F4A7C15ULL;

// All Zobrist bit-fiddling lives here -- callers never touch the random
// tables directly, they just say what changed (a piece moved, castling
// rights changed, ...) so the indexing scheme can't be gotten wrong at a
// call site.
class Zobrist {
  uint64_t piece[12][64];
  uint64_t castling[16];
  uint64_t enPassant[8];
  uint64_t sideToMove;

public:
  Zobrist() {
    std::mt19937_64 gen(ZOBRIST_SEED);
    std::uniform_int_distribution<uint64_t> dist;

    for(auto &row: piece)
      for(auto &v: row) v = dist(gen);
    for(auto &v: castling) v = dist(gen);
    for(auto &v: enPassant) v = dist(gen);
    sideToMove = dist(gen);
  }

  // XORs `p` in/out of square (x, y). No-op for EMPTY, so callers don't need
  // their own "is there actually a piece here" check.
  void togglePiece(uint64_t &hash, int p, int x, int y) const {
    if(p == EMPTY) return;
    hash ^= piece[p][bitboard.grid2bit(x, y)];
  }

  void toggleSideToMove(uint64_t &hash) const {
    hash ^= sideToMove;
  }

  // castlingPreserved is a 4-bit value covering all castling rights at once,
  // so a rights change is always "remove the old combination, add the new
  // one" rather than a per-right toggle.
  void updateCastling(uint64_t &hash, int oldRights, int newRights) const {
    hash ^= castling[oldRights];
    hash ^= castling[newRights];
  }

  // En passant is sparse (usually "no square"), unlike castling rights --
  // {-1, -1} means none and contributes nothing to the hash.
  void updateEnPassant(uint64_t &hash, i2 oldEnPassant, i2 newEnPassant) const {
    if(oldEnPassant.first != -1) hash ^= enPassant[oldEnPassant.first];
    if(newEnPassant.first != -1) hash ^= enPassant[newEnPassant.first];
  }

  // Full from-scratch computation, used to seed the very first position and
  // as the ground truth for the debug-only incremental-update sanity check
  // (see ZOBRIST_VERIFY in Game.cpp).
  uint64_t compute(const std::vector<std::vector<int>> &board, int castlingRights, i2 enPassantSq, bool whiteToMove) const {
    uint64_t hash = 0;

    for(int x=0;x<8;x++) {
      for(int y=0;y<8;y++) {
        togglePiece(hash, board[x][y], x, y);
      }
    }

    hash ^= castling[castlingRights];
    updateEnPassant(hash, {-1, -1}, enPassantSq);
    if(!whiteToMove) toggleSideToMove(hash);

    return hash;
  }
};

// const global -> internal linkage per translation unit (same pattern as
// `bitboard` in Bitboard.hpp), so including this header from multiple .cpp
// files never causes a multiple-definition link error.
const Zobrist ZOBRIST;

#endif
