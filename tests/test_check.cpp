#include "test_framework.hpp"
#include <Game.hpp>
#include "position_builder.hpp"

TEST(fools_mate) {
  Game g;
  g.doAction({5,6},{5,5}); // f2-f3
  g.doAction({4,1},{4,3}); // e7-e5
  g.doAction({6,6},{6,4}); // g2-g4
  g.doAction({3,0},{7,4}); // Qd8-h4#

  CHECK(g.isCheckMate());
  CHECK(!g.isDraw());
  CHECK_NEAR(g.getScore(), -1000.0, 1e-9); // white to move, white is mated
}

TEST(back_rank_mate) {
  Game g;
  TestPositionBuilder::setup(g, {
    {4,7, WK},
    {0,1, WR},
    {4,0, BK},
    {3,1, BP}, {4,1, BP}, {5,1, BP}, // black's own pawns seal off rank 1
  }, true);

  CHECK(g.isAvailable({0,1},{0,0}));
  g.doAction({0,1},{0,0}); // Ra7-a8+

  CHECK(g.isCheckMate());
  CHECK_NEAR(g.getScore(), 1000.0, 1e-9); // black to move, black is mated
}

TEST(pinned_piece_cannot_move_off_the_pin_line) {
  Game g;
  TestPositionBuilder::setup(g, {
    {4,7, WK},
    {4,0, BK},
    {4,1, BB}, // pinned bishop
    {4,4, WR}, // pins the bishop to the black king along the e-file
  }, false);

  CHECK(!g.isAvailable({4,1},{3,2})); // Be7-d6 would expose the king
  CHECK(!g.hasMoveFor({4,1}));
  CHECK(!g.isCheckMate());
  CHECK(!g.isDraw());
}

TEST(king_cannot_move_into_an_attacked_square) {
  Game g;
  TestPositionBuilder::setup(g, {
    {0,0, WK},
    {7,7, BK},
    {2,0, BR}, // covers all of rank 0
  }, true);

  CHECK(!g.isAvailable({0,0},{1,0}));
}
