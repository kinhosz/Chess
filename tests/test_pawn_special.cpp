#include "test_framework.hpp"
#include <Game.hpp>

TEST(pawn_double_push_only_from_starting_row) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({4,1},{4,3}); // e7-e5
  CHECK(!g.isAvailable({4,4},{4,2})); // e4-e6, no longer on the starting row
}

TEST(pawn_blocked_straight_ahead_cannot_advance) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({4,1},{4,3}); // e7-e5
  CHECK(!g.isAvailable({4,4},{4,3})); // e4-e5 is occupied by black's pawn
}

TEST(en_passant_capture) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({7,1},{7,2}); // h7-h6 (filler)
  g.doAction({4,4},{4,3}); // e4-e5
  g.doAction({3,1},{3,3}); // d7-d5 (double push next to the e5 pawn)

  CHECK(g.isAvailable({4,3},{3,2})); // e5xd6 en passant
  g.doAction({4,3},{3,2});

  CHECK_EQ(g.getBoard()[3][2], WP);   // white pawn landed on d6
  CHECK_EQ(g.getBoard()[3][3], EMPTY); // the black pawn that double-pushed is gone
  CHECK_EQ(g.getBoard()[4][3], EMPTY); // the capturing pawn's old square is empty
}

TEST(en_passant_only_available_immediately_after_the_double_push) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({7,1},{7,2}); // h7-h6
  g.doAction({4,4},{4,3}); // e4-e5
  g.doAction({3,1},{3,3}); // d7-d5
  g.doAction({0,6},{0,5}); // a2-a3 (white lets the en passant window pass)
  g.doAction({7,2},{7,3}); // h6-h5 (black, any legal move)

  CHECK(!g.isAvailable({4,3},{3,2})); // en passant no longer available
}

TEST(pawn_promotion_via_capture) {
  Game g;
  // Knight tour clears g8 and h7 so the h-pawn can promote by capturing on g8.
  g.doAction({6,7},{7,5}); // Ng1-h3
  g.doAction({0,1},{0,2}); // a7-a6
  g.doAction({7,5},{6,3}); // Nh3-g5
  g.doAction({1,1},{1,2}); // b7-b6
  g.doAction({6,3},{7,1}); // Ng5xh7
  g.doAction({2,1},{2,2}); // c7-c6
  g.doAction({7,1},{5,0}); // Nh7xf8
  g.doAction({3,1},{3,2}); // d7-d6
  g.doAction({7,6},{7,4}); // h2-h4
  g.doAction({0,2},{0,3}); // a6-a5
  g.doAction({7,4},{7,3}); // h4-h5
  g.doAction({1,2},{1,3}); // b6-b5
  g.doAction({7,3},{7,2}); // h5-h6
  g.doAction({2,2},{2,3}); // c6-c5
  g.doAction({7,2},{7,1}); // h6-h7
  g.doAction({3,2},{3,3}); // d6-d5

  CHECK(g.isPawnPromotion({7,1},{6,0}));
  g.doAction({7,1},{6,0}, 0); // h7xg8=Q

  CHECK_EQ(g.getBoard()[6][0], WQ);
}

TEST(pawn_promotion_to_each_piece_choice) {
  int choices[] = {0, 1, 2, 3};
  int expected[] = {WQ, WR, WN, WB};

  for(int c=0;c<4;c++) {
    Game g;
    g.doAction({6,7},{7,5}); g.doAction({0,1},{0,2});
    g.doAction({7,5},{6,3}); g.doAction({1,1},{1,2});
    g.doAction({6,3},{7,1}); g.doAction({2,1},{2,2});
    g.doAction({7,1},{5,0}); g.doAction({3,1},{3,2});
    g.doAction({7,6},{7,4}); g.doAction({0,2},{0,3});
    g.doAction({7,4},{7,3}); g.doAction({1,2},{1,3});
    g.doAction({7,3},{7,2}); g.doAction({2,2},{2,3});
    g.doAction({7,2},{7,1}); g.doAction({3,2},{3,3});
    g.doAction({7,1},{6,0}, choices[c]);

    CHECK_EQ(g.getBoard()[6][0], expected[c]);
  }
}
