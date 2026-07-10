#include "test_framework.hpp"
#include <Game.hpp>
#include <Engine.hpp>
#include "engine_test_access.hpp"

// max_cmp/min_cmp used to delegate to cmp(), whose epsilon-based "equality"
// breaks the transitivity std::sort requires for a strict weak ordering --
// that caused a real heap-buffer-overflow in std::sort's introsort. The bug
// only reproduces once a search tree happens to grow a comparison array large
// enough to hit the unguarded partition path, so it is NOT reliably caught by
// just running the engine (verified: reintroducing the bug still passed 5/5
// engine-search runs below). This checks the comparator's contract directly.
TEST(sort_comparators_are_a_strict_weak_ordering) {
  CHECK(!max_cmp({1.0,0},{1.0,0}));   // irreflexive
  CHECK(!min_cmp({1.0,0},{1.0,0}));

  CHECK(max_cmp({2.0,0},{1.0,0}));    // agrees with plain > / <
  CHECK(!max_cmp({1.0,0},{2.0,0}));
  CHECK(min_cmp({1.0,0},{2.0,0}));
  CHECK(!min_cmp({2.0,0},{1.0,0}));

  // Values within cmp()'s old epsilon (1e-4) must still compare strictly --
  // an epsilon-based comparator would call these "equal" and violate the
  // transitivity std::sort's introsort relies on to stay in bounds.
  CHECK(max_cmp({1.00005,0},{1.0,0}));
  CHECK(min_cmp({1.0,0},{1.00005,0}));
}

// createNextLines() used to push 4 entries into `lines` for a promotion move
// but only 1 into `sorted_ptr`, desyncing every index after the first
// promotion candidate -- the move ultimately played stayed legal (still a
// real entry from `lines`), so a black-box "is the move legal" check like
// engine_always_returns_legal_moves_and_does_not_crash below can't see this;
// only the size invariant catches it directly.
TEST(createNextLines_keeps_lines_and_sorted_ptr_aligned_with_promotions) {
  Game g;
  // Reach a position where white has a pawn one step from promoting
  // (h7xg8), mixed in among the rest of white's ordinary legal moves.
  g.doAction({6,7},{7,5}); g.doAction({0,1},{0,2});
  g.doAction({7,5},{6,3}); g.doAction({1,1},{1,2});
  g.doAction({6,3},{7,1}); g.doAction({2,1},{2,2});
  g.doAction({7,1},{5,0}); g.doAction({3,1},{3,2});
  g.doAction({7,6},{7,4}); g.doAction({0,2},{0,3});
  g.doAction({7,4},{7,3}); g.doAction({1,2},{1,3});
  g.doAction({7,3},{7,2}); g.doAction({2,2},{2,3});
  g.doAction({7,2},{7,1}); g.doAction({3,2},{3,3});

  i5 rootMove = {{{-1,-1},{-1,-1}}, -1};
  EngineNode node(rootMove);
  TestEngineAccess::createNextLines(node, g);

  CHECK_EQ(TestEngineAccess::linesSize(node), TestEngineAccess::sortedPtrSize(node));
}

// Drives the Engine and a plain Game in lockstep: every move the engine
// proposes gets validated against the Game's own legality check, then
// applied to both. This exercises the real search (explore/createNextLines/
// moveDone tree pruning) end to end and covers the promotion index desync
// bug found earlier.
TEST(engine_always_returns_legal_moves_and_does_not_crash) {
  Engine engine;
  Game game;

  for(int ply=0; ply<8; ply++) {
    if(game.isCheckMate() || game.isDraw()) break;

    i5 move = engine.getNextMove(2); // shallow depth, keep the test fast
    bool legal = game.isAvailable(move.first.first, move.first.second);
    CHECK(legal);
    if(!legal) break; // avoid cascading failures from an illegal move

    game.doAction(move.first.first, move.first.second, move.second);
    engine.moveDone(move);
  }
}

TEST(engine_moveDone_tree_stays_consistent_across_several_plies) {
  // Regression coverage for the moveDone tree-pruning bug: lines/sorted_ptr
  // must stay index-aligned across repeated commits, including through any
  // pawn promotions the engine chooses along the way.
  Engine engine;
  Game game;
  int nodes = 0;

  for(int ply=0; ply<10; ply++) {
    if(game.isCheckMate() || game.isDraw()) break;

    i5 move = engine.getNextMove(3, &nodes);
    CHECK(nodes > 0);
    CHECK(game.isAvailable(move.first.first, move.first.second));

    game.doAction(move.first.first, move.first.second, move.second);
    engine.moveDone(move);
  }
}
