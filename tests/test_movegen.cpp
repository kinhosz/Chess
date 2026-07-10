#include "test_framework.hpp"
#include <Game.hpp>

TEST(bishop_is_blocked_by_own_pawns_at_start) {
  Game g;
  CHECK(!g.isAvailable({2,7},{1,6})); // Bc1-b2, own pawn in the way
  CHECK(!g.isAvailable({2,7},{3,6})); // Bc1-d2, own pawn in the way
}

TEST(pawn_can_capture_diagonally) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({3,1},{3,3}); // d7-d5
  CHECK(g.isAvailable({4,4},{3,3})); // exd5
  g.doAction({4,4},{3,3});
  CHECK_EQ(g.getBoard()[3][3], WP);
  CHECK_EQ(g.getBoard()[4][4], EMPTY);
}

TEST(rook_move_blocked_beyond_first_occupied_square) {
  Game g;
  g.doAction({0,6},{0,4}); // a2-a4, opens the file for the rook
  g.doAction({0,1},{0,2}); // a7-a6 (black filler)
  CHECK(g.isAvailable({0,7},{0,5})); // Ra1-a3
  CHECK(g.isAvailable({0,7},{0,6})); // Ra1-a2
  CHECK(!g.isAvailable({0,7},{0,3})); // Ra1-a5, own pawn on a4 blocks further travel
}

TEST(knight_move_shape_is_correct) {
  Game g;
  CHECK(!g.isAvailable({1,7},{1,5})); // Nb1-b3 is not a knight-shaped move
  CHECK(!g.isAvailable({1,7},{4,7})); // Nb1-e1 is not a knight-shaped move
}
