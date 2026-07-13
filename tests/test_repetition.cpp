#include "test_framework.hpp"
#include <Game.hpp>

// These tests define the CORRECT repetition semantics (same piece placement +
// same side to move + same castling rights + same en passant rights = same
// position). The current Game::getBoardHash() only hashes piece placement,
// so tests below that vary castling/en-passant while keeping placement equal
// are expected to fail until the Zobrist hash folds those in too. Use this
// file as the acceptance check for that migration.

TEST(twofold_repetition_is_not_a_draw) {
  Game g;
  g.doAction({1,7},{2,5}); // Nb1-c3
  g.doAction({1,0},{2,2}); // Nb8-c6
  g.doAction({2,5},{1,7}); // Nc3-b1
  g.doAction({2,2},{1,0}); // Nc6-b8

  CHECK(!g.isDraw());
}

TEST(repetition_must_not_ignore_lost_castling_rights) {
  Game g;
  g.doAction({1,7},{0,5}); // Nb1-a3 (clears b1 so the rook can shuffle)

  // One full cycle: reach {WN@a3, BN@b8, WR@a1} with white's queenside
  // castling right now lost (Ra1-b1 touches it). Placement matches the
  // position right after Nb1-a3 above, but it is NOT the same chess
  // position anymore.
  g.doAction({1,0},{2,2}); // Nb8-c6
  g.doAction({0,7},{1,7}); // Ra1-b1 (touches white queenside castling right)
  g.doAction({2,2},{1,0}); // Nc6-b8
  g.doAction({1,7},{0,7}); // Rb1-a1

  // Cycle again: this is only the 2nd true occurrence of the
  // rights-lost position (the pre-cycle occurrence had rights intact, so it
  // must not count toward this position's repetition total).
  g.doAction({1,0},{2,2}); // Nb8-c6
  g.doAction({0,7},{1,7}); // Ra1-b1
  g.doAction({2,2},{1,0}); // Nc6-b8
  g.doAction({1,7},{0,7}); // Rb1-a1

  CHECK(!g.isDraw());

  // Cycle a third time: now the rights-lost position has genuinely occurred
  // three times and this must be a draw.
  g.doAction({1,0},{2,2}); // Nb8-c6
  g.doAction({0,7},{1,7}); // Ra1-b1
  g.doAction({2,2},{1,0}); // Nc6-b8
  g.doAction({1,7},{0,7}); // Rb1-a1

  CHECK(g.isDraw());
}

TEST(repetition_must_not_ignore_en_passant_availability) {
  Game g;
  g.doAction({3,6},{3,4}); // d2-d4 (double push, en passant becomes available)

  // Shuffle knights back and forth without touching the d-pawn. Each full
  // cycle restores the exact placement reached right after d2-d4, but only
  // the very first occurrence had en passant actually available -- later
  // occurrences are a different position and must not be conflated with it.
  for(int i=0;i<2;i++) {
    g.doAction({1,7},{2,5}); // Nb1-c3
    g.doAction({1,0},{2,2}); // Nb8-c6
    g.doAction({2,5},{1,7}); // Nc3-b1
    g.doAction({2,2},{1,0}); // Nc6-b8
  }

  CHECK(!g.isDraw());

  g.doAction({1,7},{2,5}); // Nb1-c3
  g.doAction({1,0},{2,2}); // Nb8-c6
  g.doAction({2,5},{1,7}); // Nc3-b1
  g.doAction({2,2},{1,0}); // Nc6-b8

  CHECK(g.isDraw());
}
