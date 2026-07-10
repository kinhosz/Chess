#include "test_framework.hpp"
#include <Game.hpp>

TEST(initial_position_is_white_to_move) {
  Game g;
  CHECK(g.isWhiteTurn());
  CHECK(!g.isCheckMate());
  CHECK(!g.isDraw());
  CHECK_EQ(g.getTotalMoves(), 0);
}

TEST(initial_position_has_twenty_legal_moves) {
  Game g;
  // 8 pawns x 2 (single + double push) + 2 knights x 2 jumps each = 20
  CHECK_EQ((int)g.genNextMoves().size(), 20);
}

TEST(initial_position_score_is_symmetric) {
  Game g;
  CHECK_NEAR(g.getScore(), 0.0, 1e-9);
}

TEST(initial_position_back_rank_pieces_are_boxed_in) {
  Game g;
  // Rook, bishop, queen and king are fully blocked by their own pawns/pieces.
  CHECK(!g.hasMoveFor({0,7})); // Ra1
  CHECK(!g.hasMoveFor({2,7})); // Bc1
  CHECK(!g.hasMoveFor({3,7})); // Qd1
  CHECK(!g.hasMoveFor({4,7})); // Ke1
}

TEST(initial_position_knights_have_exactly_two_jumps) {
  Game g;
  CHECK(g.isAvailable({1,7},{0,5})); // Nb1-a3
  CHECK(g.isAvailable({1,7},{2,5})); // Nb1-c3
  CHECK(!g.isAvailable({1,7},{3,6})); // Nb1-d2 -- blocked by own pawn
}

TEST(empty_square_and_opponent_piece_have_no_moves_for_side_to_move) {
  Game g;
  CHECK(!g.hasMoveFor({4,4})); // empty square
  CHECK(!g.hasMoveFor({4,1})); // black pawn, but it's white's turn
}
