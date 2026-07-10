#ifndef TEST_ENGINE_ACCESS_HPP
#define TEST_ENGINE_ACCESS_HPP

#include <Engine.hpp>

// Test-only helper (friend of EngineNode, see the friend declaration in
// include/Engine.hpp) that inspects lines/sorted_ptr directly. Lets tests
// check they stay index-aligned through createNextLines() without depending
// on the search happening to expose a misattributed score via move choice.
struct TestEngineAccess {
  static void createNextLines(EngineNode &node, Game &game) {
    node.createNextLines(game);
  }

  static size_t linesSize(const EngineNode &node) { return node.lines.size(); }
  static size_t sortedPtrSize(const EngineNode &node) { return node.sorted_ptr.size(); }
};

#endif
