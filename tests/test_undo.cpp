#include "test_framework.hpp"
#include <Game.hpp>

TEST(undo_restores_board_and_turn) {
  Game g;
  auto before = g.getBoard();

  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({4,1},{4,3}); // e7-e5
  g.undoAction();
  g.undoAction();

  CHECK(before == g.getBoard());
  CHECK(g.isWhiteTurn());
  CHECK_EQ(g.getTotalMoves(), 0);
}

TEST(undo_restores_score) {
  Game g;
  double scoreBefore = g.getScore();

  g.doAction({4,6},{4,4});
  g.doAction({4,1},{4,3});
  g.undoAction();
  g.undoAction();

  CHECK_NEAR(g.getScore(), scoreBefore, 1e-9);
}

TEST(undo_restores_a_captured_piece) {
  Game g;
  auto before = g.getBoard();

  g.doAction({4,6},{4,4}); // e2-e4
  g.doAction({3,1},{3,3}); // d7-d5
  g.doAction({4,4},{3,3}); // exd5 (capture)
  g.undoAction();
  g.undoAction();
  g.undoAction();

  CHECK(before == g.getBoard());
}

TEST(undo_after_castling_restores_king_and_rook) {
  Game g;
  g.doAction({4,6},{4,4}); g.doAction({7,1},{7,2});
  g.doAction({4,4},{4,3}); g.doAction({3,1},{3,3});
  g.doAction({4,3},{3,2}); // en passant, just to reuse a verified line
  g.doAction({1,0},{2,2}); g.doAction({6,7},{5,5});
  g.doAction({6,1},{6,2}); g.doAction({5,7},{4,6});
  g.doAction({0,1},{0,2});

  auto beforeCastle = g.getBoard();
  g.doAction({4,7},{6,7}); // O-O
  g.undoAction();

  CHECK(beforeCastle == g.getBoard());
}
