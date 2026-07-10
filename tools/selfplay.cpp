#include <iostream>
#include <chrono>
#include <cstdlib>
#include <string>

#include <Game.hpp>
#include <Engine.hpp>
#include <Profiler.hpp>

// Headless self-play harness: drives the Engine against itself with no SFML
// dependency, so it can be compiled with -fsanitize=address,undefined
// (`make selfplay-debug`) to catch search bugs that only surface once a real
// search tree is built (out-of-bounds reads, corrupted move indices, etc).
// Also reuses Profiler (per-function time breakdown + RSS) so search
// performance can be inspected without going through the UI.
//
// usage: selfplay [depth] [max_moves] [--quiet-profiler] [--explain]

void printExplanation(const MoveExplanation &exp, int topN) {
  std::cerr << "  top candidates:\n";
  for(int i=0;i<topN && i<(int)exp.candidates.size();i++) {
    const auto &c = exp.candidates[i];
    std::cerr << "    " << (i+1) << ". " << squareName(c.move.first.first) << squareName(c.move.first.second)
               << " promo=" << c.move.second << "  score=" << c.score << "\n";
  }
  std::cerr << "  expected continuation:";
  for(auto &m: exp.principalVariation) {
    std::cerr << " " << squareName(m.first.first) << squareName(m.first.second);
  }
  std::cerr << "\n";
}

int main(int argc, char** argv) {
  if(argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help")) {
    std::cerr << "usage: " << argv[0] << " [depth=5] [max_moves=40] [--quiet-profiler] [--explain]\n";
    return 0;
  }

  int depth = argc > 1 ? std::atoi(argv[1]) : 5;
  int max_moves = argc > 2 ? std::atoi(argv[2]) : 40;
  bool quiet_profiler = false;
  bool explain = false;
  for(int i=3;i<argc;i++) {
    std::string arg = argv[i];
    if(arg == "--quiet-profiler") quiet_profiler = true;
    else if(arg == "--explain") explain = true;
  }

  Game game;
  Engine engine;

  long long total_nodes = 0;
  auto game_start = std::chrono::steady_clock::now();

  int m = 0;
  for(; m < max_moves; m++) {
    if(game.isCheckMate() || game.isDraw()) break;

    int nodes = 0;
    MoveExplanation explanation;
    auto t0 = std::chrono::steady_clock::now();
    i5 mv = engine.getNextMove(depth, &nodes, explain ? &explanation : nullptr);
    auto t1 = std::chrono::steady_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    total_nodes += nodes;

    std::cerr << "move " << m << " (" << (game.isWhiteTurn() ? "white" : "black") << "): ("
               << mv.first.first.first << "," << mv.first.first.second << ") -> ("
               << mv.first.second.first << "," << mv.first.second.second << ")"
               << " promo=" << mv.second
               << "  nodes=" << nodes
               << "  time=" << ms << "ms";

    game.doAction(mv.first.first, mv.first.second, mv.second);
    engine.moveDone(mv);

    std::cerr << "  score=" << game.getScore() << "\n";

    if(explain) printExplanation(explanation, 5);

    if(!quiet_profiler) {
      Profiler::getInstance().logAll();
      Profiler::getInstance().logMemory("after move " + std::to_string(m));
    }
  }

  auto game_end = std::chrono::steady_clock::now();
  double total_s = std::chrono::duration<double>(game_end - game_start).count();

  if(game.isCheckMate()) std::cerr << "GAME OVER: checkmate after " << m << " moves\n";
  else if(game.isDraw()) std::cerr << "GAME OVER: draw after " << m << " moves\n";
  else std::cerr << "stopped after reaching move limit (" << max_moves << ")\n";

  std::cerr << "final score: " << game.getScore() << "\n";
  std::cerr << "total time: " << total_s << "s across " << m << " moves ("
            << (m ? total_s / m : 0.0) << "s/move avg)\n";
  std::cerr << "total nodes: " << total_nodes
            << " (" << (total_s > 0 ? total_nodes / total_s : 0.0) << " nodes/s)\n";

  return 0;
}
