#include "test_framework.hpp"
#include <Game.hpp>
#include "position_builder.hpp"

TEST(white_kingside_castling) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({7,1},{7,2}); // h7-h6
  g.doAction({4,4},{4,3}); // e4-e5
  g.doAction({3,1},{3,3}); // d7-d5
  g.doAction({4,3},{3,2}); // e5xd6 (en passant, just to reuse a verified line)
  g.doAction({1,0},{2,2}); // Nb8-c6
  g.doAction({6,7},{5,5}); // Ng1-f3
  g.doAction({6,1},{6,2}); // g7-g6
  g.doAction({5,7},{4,6}); // Bf1-e2
  g.doAction({0,1},{0,2}); // a7-a6

  CHECK(g.isAvailable({4,7},{6,7}));
  g.doAction({4,7},{6,7}); // O-O

  CHECK_EQ(g.getBoard()[6][7], WK);
  CHECK_EQ(g.getBoard()[5][7], WR);
  CHECK_EQ(g.getBoard()[4][7], EMPTY);
  CHECK_EQ(g.getBoard()[7][7], EMPTY);
}

TEST(black_kingside_castling) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({4,1},{4,3}); // e7-e5
  g.doAction({6,7},{5,5}); // Ng1-f3
  g.doAction({6,0},{5,2}); // Ng8-f6
  g.doAction({1,7},{2,5}); // Nb1-c3
  g.doAction({5,0},{4,1}); // Bf8-e7
  g.doAction({3,6},{3,4}); // d2-d4

  CHECK(g.isAvailable({4,0},{6,0}));
  g.doAction({4,0},{6,0}); // O-O

  CHECK_EQ(g.getBoard()[6][0], BK);
  CHECK_EQ(g.getBoard()[5][0], BR);
  CHECK_EQ(g.getBoard()[4][0], EMPTY);
  CHECK_EQ(g.getBoard()[7][0], EMPTY);
}

TEST(white_queenside_castling) {
  Game g;
  g.doAction({1,7},{2,5}); // Nb1-c3
  g.doAction({0,1},{0,2}); // a7-a6
  g.doAction({3,6},{3,4}); // d2-d4
  g.doAction({0,2},{0,3}); // a6-a5
  g.doAction({2,7},{5,4}); // Bc1-f4
  g.doAction({0,3},{0,4}); // a5-a4
  g.doAction({3,7},{3,6}); // Qd1-d2
  g.doAction({0,4},{0,5}); // a4-a3

  CHECK(g.isAvailable({4,7},{2,7}));
  g.doAction({4,7},{2,7}); // O-O-O

  CHECK_EQ(g.getBoard()[2][7], WK);
  CHECK_EQ(g.getBoard()[3][7], WR);
  CHECK_EQ(g.getBoard()[0][7], EMPTY);
  CHECK_EQ(g.getBoard()[4][7], EMPTY);
}

TEST(castling_rights_are_lost_permanently_once_the_king_moves) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4, clears e2
  g.doAction({0,1},{0,2}); // a7-a6
  g.doAction({4,7},{4,6}); // Ke1-e2
  g.doAction({0,2},{0,3}); // a6-a5
  g.doAction({4,6},{4,7}); // Ke2-e1, king is back home
  g.doAction({0,3},{0,4}); // a5-a4
  g.doAction({6,7},{5,5}); // Ng1-f3, clears g1
  g.doAction({0,4},{0,5}); // a4-a3
  g.doAction({5,7},{4,6}); // Bf1-e2, clears f1
  g.doAction({7,1},{7,2}); // h7-h6

  // Squares are clear and the king isn't in check, but the right is gone.
  CHECK(!g.isAvailable({4,7},{6,7}));
}

TEST(castling_blocked_by_own_piece_in_the_way) {
  Game g;
  // Nothing has moved yet: f1/g1 and b1/c1/d1 are all still occupied.
  CHECK(!g.isAvailable({4,7},{6,7}));
  CHECK(!g.isAvailable({4,7},{2,7}));
}

TEST(castling_illegal_while_in_check) {
  Game g;
  TestPositionBuilder::setup(g, {
    {4,7, WK}, {0,7, WR}, {7,7, WR},
    {4,0, BK}, {4,4, BR}, // checks the white king along the e-file
  }, true);

  CHECK(!g.isAvailable({4,7},{6,7}));
  CHECK(!g.isAvailable({4,7},{2,7}));
}

TEST(castling_illegal_through_an_attacked_square) {
  Game g;
  TestPositionBuilder::setup(g, {
    {4,7, WK}, {7,7, WR},
    {4,0, BK}, {1,3, BB}, // bishop on b5 attacks f1 along the diagonal
  }, true);

  CHECK(!g.isAvailable({4,7},{6,7}));
}
