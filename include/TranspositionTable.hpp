#ifndef TRANSPOSITION_TABLE_HPP
#define TRANSPOSITION_TABLE_HPP

#include <cstdint>
#include <cstdlib>
#include <vector>
#include <Define.hpp>

// Fail-soft alpha-beta (see EngineNode::explore in Engine.hpp) only proves an
// EXACT value when the search stays inside the [alpha, beta] window it was
// given; a cutoff only proves a LOWER bound (fail-high, real value >= score)
// or an UPPER bound (fail-low, real value <= score). Reusing a bound as if it
// were exact would corrupt later cutoff decisions, so the table must keep
// track of which kind each stored score is.
enum class Bound : uint8_t { EXACT, LOWER, UPPER };

struct TTEntry {
  uint64_t hash = 0;
  int depth = -1; // -1 marks an empty slot
  Score score = 0;
  Bound bound = Bound::EXACT;
};

// Same getenv-based config pattern as CHESS_SEED (see chessRngSeed() in
// Engine.hpp): lets the table size be tuned, and CHESS_TT_SIZE=0 disables the
// table entirely -- used by the A/B correctness check to compare searches
// with and without transposition reuse without a separate code path.
inline size_t chessTtEntries() {
  const char *env = std::getenv("CHESS_TT_SIZE");
  if(env) return std::strtoull(env, nullptr, 10);
  return 1ull << 22; // ~96MB at 24 bytes/entry
}

// Fixed-size, indexed by hash & mask (never grows) -- an unbounded map would
// fight the multi-GB peak RSS this engine already has at depth 7. The full
// hash is kept per entry so an index collision (different position, same
// slot) is always detected as a miss, never returned as a wrong hit.
class TranspositionTable {
  std::vector<TTEntry> table;
  uint64_t mask;

  static size_t roundUpPow2(size_t v) {
    size_t p = 1;
    while(p < v) p <<= 1;
    return p;
  }

public:
  explicit TranspositionTable(size_t entries = chessTtEntries()) {
    size_t size = entries == 0 ? 0 : roundUpPow2(entries);
    table.resize(size);
    mask = size == 0 ? 0 : size - 1;
  }

  // nullptr on miss (empty slot, hash mismatch, or table disabled).
  const TTEntry *probe(uint64_t hash) const {
    if(table.empty()) return nullptr;
    const TTEntry &e = table[hash & mask];
    if(e.depth == -1 || e.hash != hash) return nullptr;
    return &e;
  }

  // Depth-preferred replacement: a deeper existing entry is more expensive to
  // recompute than whatever just finished, so it's kept regardless of
  // whether it's the same position -- shallow entries are cheap to redo.
  void store(uint64_t hash, int depth, Score score, Bound bound) {
    if(table.empty()) return;
    TTEntry &e = table[hash & mask];
    if(e.depth != -1 && e.depth > depth) return;
    e = {hash, depth, score, bound};
  }
};

#endif
