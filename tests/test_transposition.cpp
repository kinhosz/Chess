#include "test_framework.hpp"
#include <cstdlib>
#include <Engine.hpp>
#include <TranspositionTable.hpp>

TEST(empty_table_probe_misses) {
  TranspositionTable tt(1024);
  CHECK(tt.probe(12345) == nullptr);
}

TEST(store_then_probe_roundtrip) {
  TranspositionTable tt(1024);
  tt.store(0xABCDEF, 5, 100, Bound::LOWER);

  const TTEntry *e = tt.probe(0xABCDEF);
  CHECK(e != nullptr);
  CHECK_EQ(e->depth, 5);
  CHECK_EQ(e->score, 100);
  CHECK(e->bound == Bound::LOWER);
}

TEST(depth_preferred_replacement_keeps_the_deeper_entry) {
  TranspositionTable tt(1024);
  tt.store(1, 5, 100, Bound::EXACT);
  tt.store(1, 3, 200, Bound::EXACT); // shallower -- must not evict depth 5

  const TTEntry *e = tt.probe(1);
  CHECK(e != nullptr);
  CHECK_EQ(e->depth, 5);
  CHECK_EQ(e->score, 100);

  tt.store(1, 7, 300, Bound::EXACT); // deeper -- must replace

  e = tt.probe(1);
  CHECK(e != nullptr);
  CHECK_EQ(e->depth, 7);
  CHECK_EQ(e->score, 300);
}

TEST(disabled_table_never_stores_or_returns_a_hit) {
  TranspositionTable tt(0);
  tt.store(42, 10, 999, Bound::EXACT);
  CHECK(tt.probe(42) == nullptr);
}

TEST(hash_mismatch_in_the_same_slot_is_a_miss_not_a_wrong_hit) {
  TranspositionTable tt(2); // rounds up to 2 entries, mask = 1
  tt.store(0, 5, 100, Bound::EXACT);  // slot 0 & 1 = 0
  CHECK(tt.probe(2) == nullptr);      // slot 2 & 1 = 0 too, different hash
}

// The Zobrist-phase invariant ("node counts identical before/after") doesn't
// apply here -- the whole point of the TT is to change node counts. The
// invariant that must hold instead: reusing a transposition never changes
// which move is chosen or what score it gets, only how many nodes it took to
// get there. CHESS_TT_SIZE=0 (see chessTtEntries() in TranspositionTable.hpp)
// gives an A/B toggle without a second code path to keep in sync.
TEST(transposition_table_does_not_change_chosen_moves_or_scores) {
  setenv("CHESS_TT_SIZE", "0", 1);
  Engine engineDisabled;

  unsetenv("CHESS_TT_SIZE");
  Engine engineEnabled;

  const int depth = 4;
  const int plies = 6;
  int totalNodesDisabled = 0, totalNodesEnabled = 0;

  for(int ply=0; ply<plies; ply++) {
    int nodesDisabled = 0, nodesEnabled = 0;
    i5 moveDisabled = engineDisabled.getNextMove(depth, &nodesDisabled);
    i5 moveEnabled = engineEnabled.getNextMove(depth, &nodesEnabled);

    CHECK(moveDisabled == moveEnabled);

    totalNodesDisabled += nodesDisabled;
    totalNodesEnabled += nodesEnabled;

    engineDisabled.moveDone(moveDisabled);
    engineEnabled.moveDone(moveEnabled);
  }

  CHECK(totalNodesEnabled <= totalNodesDisabled);
}
