#include "test_framework.hpp"
#include <Game.hpp>
#include "position_builder.hpp"

TEST(checkmate_score_is_plus_or_minus_1000) {
  {
    Game g; // white delivers mate -> good for white -> +CHECKMATE_SCORE (black to move)
    g.doAction({5,6},{5,5});
    g.doAction({4,1},{4,3});
    g.doAction({6,6},{6,4});
    g.doAction({3,0},{7,4});
    CHECK_EQ(g.getScore(), -CHECKMATE_SCORE); // score is from the side-to-move's perspective (white, mated)
  }
  {
    Game g;
    TestPositionBuilder::setup(g, {
      {4,7, WK}, {0,1, WR}, {4,0, BK}, {3,1, BP}, {4,1, BP}, {5,1, BP},
    }, true);
    g.doAction({0,1},{0,0}); // white mates black
    CHECK_EQ(g.getScore(), CHECKMATE_SCORE); // black to move, black is mated
  }
}

TEST(draw_score_is_zero) {
  Game g;
  TestPositionBuilder::setup(g, {{0,4,WK},{7,7,BK}}, true);
  CHECK_EQ(g.getScore(), 0);
}

TEST(capturing_a_piece_changes_the_score_by_its_value) {
  Game g;
  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({3,1},{3,3}); // d7-d5
  Score before = g.getScore();
  g.doAction({4,4},{3,3}); // exd5, wins a pawn for white
  Score after = g.getScore();

  CHECK(after > before); // capturing a pawn should improve white's score
}

TEST(get_cell_score_matches_material_value) {
  Game g;
  CHECK_EQ(g.getCellScore(3,7), PIECE_VALUE_QUEEN);  // white queen
  CHECK_EQ(g.getCellScore(3,0), -PIECE_VALUE_QUEEN); // black queen
  CHECK_EQ(g.getCellScore(4,4), 0);  // empty square
}
