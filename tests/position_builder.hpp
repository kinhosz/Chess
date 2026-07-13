#ifndef TEST_POSITION_BUILDER_HPP
#define TEST_POSITION_BUILDER_HPP

#include <vector>

#include <Game.hpp>
#include <Zobrist.hpp>

struct PlacedPiece { int x, y, piece; };

// Test-only helper (friend of Game, see the friend declaration in
// include/Game.hpp) that drops an arbitrary set of pieces onto an otherwise
// empty board and finalizes the resulting GameState the same way
// Game::doAction() would. Lets tests reach endgame positions (insufficient
// material, stalemate) directly instead of playing out dozens of legal moves
// to get there. Every position MUST include exactly one WK and one BK.
struct TestPositionBuilder {
  static void setup(Game &g, const std::vector<PlacedPiece> &pieces, bool whiteToMove) {
    for(int x=0;x<8;x++) {
      for(int y=0;y<8;y++) {
        g.setBoard(x, y, EMPTY);
      }
    }

    for(const auto &p: pieces) {
      g.setBoard(p.x, p.y, p.piece);
    }

    GameState gs;
    gs.gameStatus = ALIVE;
    gs.enPassant = {-1, -1};
    gs.castlingPreserved = 0b1111; // no castling rights in constructed positions
    gs.piecesScoring = 0.0;
    gs.repetition = false;
    gs.pieces_counter.resize(24, 0);
    gs.castled = 0;
    gs.hasMoves = true;

    for(const auto &p: pieces) {
      int id = p.piece;
      if(id == EMPTY || id == WK || id == BK) continue;
      gs.pieces_counter[id]++;

      if(id == WB) gs.pieces_counter[(p.x%2 + p.y%2)%2 == 0 ? WB_LIGHT : WB_DARK]++;
      else if(id == BB) gs.pieces_counter[(p.x%2 + p.y%2)%2 == 0 ? BB_LIGHT : BB_DARK]++;
    }

    g.gameState.clear();
    g.moves.clear();
    g.hashedBoardCounter.clear();

    gs.zobristHash = ZOBRIST.compute(g.board, gs.castlingPreserved, gs.enPassant, whiteToMove);
    g.addState(gs);

    // isWhiteTurn() derives from moves.size() parity; a placeholder rollback
    // entry (never undone) flips it to black's turn when needed.
    if(!whiteToMove) g.moves.push_back(vi3());

    GameState &finalState = g.gameState.back();
    finalState.hasMoves = g.hasAnyMove();

    if(g.drawConditions(finalState)) finalState.gameStatus = DRAW;
    else if(!finalState.hasMoves && g.isOnCheck()) finalState.gameStatus = CHECKMATE;
  }
};

#endif
