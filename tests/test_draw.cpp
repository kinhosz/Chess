#include "test_framework.hpp"
#include <Game.hpp>
#include "position_builder.hpp"

TEST(bare_kings_is_a_draw) {
  Game g;
  TestPositionBuilder::setup(g, {{0,4,WK},{7,7,BK}}, true);
  CHECK(g.isDraw());
}

TEST(lone_minor_piece_vs_bare_king_is_a_draw) {
  {
    Game g;
    TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WB},{7,7,BK}}, true);
    CHECK(g.isDraw());
  }
  {
    Game g;
    TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WN},{7,7,BK}}, true);
    CHECK(g.isDraw());
  }
}

TEST(lone_rook_queen_or_pawn_vs_bare_king_is_not_a_draw) {
  {
    Game g;
    TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WR},{7,7,BK}}, true);
    CHECK(!g.isDraw());
  }
  {
    Game g;
    TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WQ},{7,7,BK}}, true);
    CHECK(!g.isDraw());
  }
  {
    Game g;
    TestPositionBuilder::setup(g, {{0,4,WK},{2,3,WP},{7,7,BK}}, true);
    CHECK(!g.isDraw());
  }
}

TEST(knight_vs_bishop_is_a_draw) {
  // The exact regression reported live: K+N vs K+B was wrongly treated as
  // sufficient mating material before this was fixed.
  Game g;
  TestPositionBuilder::setup(g, {{4,4,WK},{2,2,WN},{4,0,BK},{6,3,BB}}, true);
  CHECK(g.isDraw());
  CHECK_EQ(g.getScore(), 0);
}

TEST(knight_vs_knight_is_a_draw) {
  Game g;
  TestPositionBuilder::setup(g, {{4,4,WK},{2,2,WN},{4,0,BK},{6,3,BN}}, true);
  CHECK(g.isDraw());
}

TEST(two_knights_same_side_vs_bare_king_is_a_draw) {
  Game g;
  TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WN},{4,2,WN},{7,7,BK}}, true);
  CHECK(g.isDraw());
}

TEST(bishop_and_knight_same_side_vs_bare_king_is_a_draw) {
  // Accepted simplification: bishop+knight mate is real but rare enough that
  // this engine doesn't try to detect it (see the comment in drawConditions).
  Game g;
  TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WB},{4,2,WN},{7,7,BK}}, true);
  CHECK(g.isDraw());
}

TEST(bishop_pair_same_color_squares_is_a_draw) {
  Game g;
  // (2,2) and (4,2): (x+y)%2 == 0 for both -- same square color.
  TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WB},{4,2,WB},{7,7,BK}}, true);
  CHECK(g.isDraw());
}

TEST(bishop_pair_opposite_color_squares_can_force_mate) {
  Game g;
  // (2,2) parity 0, (3,2) parity 1 -- opposite square colors.
  TestPositionBuilder::setup(g, {{0,4,WK},{2,2,WB},{3,2,WB},{7,7,BK}}, true);
  CHECK(!g.isDraw());
}

TEST(rook_or_queen_alongside_a_minor_piece_is_not_a_draw) {
  Game g;
  TestPositionBuilder::setup(g, {{4,4,WK},{2,2,WR},{4,0,BK},{6,3,BB}}, true);
  CHECK(!g.isDraw());
}

TEST(stalemate_is_a_draw) {
  Game g;
  // Classic corner stalemate: Kc7 + Qb6 vs Ka8, black to move.
  TestPositionBuilder::setup(g, {{2,1,WK},{1,2,WQ},{0,0,BK}}, false);

  CHECK(!g.isCheckMate());
  CHECK(g.isDraw());
  CHECK_EQ((int)g.genNextMoves().size(), 0);
}

TEST(threefold_repetition_is_a_draw) {
  Game g;
  for(int i=0;i<2;i++) {
    g.doAction({1,7},{2,5}); // Nb1-c3
    g.doAction({1,0},{2,2}); // Nb8-c6
    g.doAction({2,5},{1,7}); // Nc3-b1
    g.doAction({2,2},{1,0}); // Nc6-b8
  }

  CHECK(g.isDraw());
  CHECK_EQ(g.getScore(), 0);
}
