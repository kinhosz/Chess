#include <iostream>
#include <sstream>
#include <string>

#include <Game.hpp>
#include <Engine.hpp>

// One side of an A/B match: a persistent process that speaks a tiny
// line-based protocol over stdin/stdout, driven by tools/match.cpp.
//
//   go <depth>              -> compute our move, apply it, reply with:
//                              "<fx> <fy> <tx> <ty> <promo>"
//   move <fx fy tx ty promo> -> apply the opponent's move (no reply)
//   quit                     -> exit
//
// Compiled twice against two different git revisions of include/Engine.hpp +
// src/Game.cpp (see scripts/match.sh) so two versions of the eval/search can
// play each other while sharing this exact driver.

int main() {
  Engine engine;
  std::string line;

  while(std::getline(std::cin, line)) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    if(cmd == "go") {
      int depth;
      iss >> depth;
      i5 move = engine.getNextMove(depth);
      engine.moveDone(move);

      std::cout << move.first.first.first << " " << move.first.first.second << " "
                 << move.first.second.first << " " << move.first.second.second << " "
                 << move.second << std::endl;
      std::cout.flush();
    } else if(cmd == "move") {
      int fx, fy, tx, ty, promo;
      iss >> fx >> fy >> tx >> ty >> promo;
      engine.moveDone({{{fx,fy},{tx,ty}}, promo});
    } else if(cmd == "quit") {
      break;
    }
  }

  return 0;
}
