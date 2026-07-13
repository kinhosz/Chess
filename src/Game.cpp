#include <Game.hpp>
#include <chrono>
#include <iomanip>
#include <Bitboard.hpp>
#include <Zobrist.hpp>
#include <Profiler.hpp>

Game::Game() {
  GameState gs;
  gs.gameStatus = ALIVE;
  gs.enPassant = {-1, -1};
  gs.castlingPreserved = 0;
  gs.piecesScoring = 0.0;
  gs.repetition = false;
  gs.pieces_counter.resize(24, 0);
  gs.castled = 0;
  gs.hasMoves = true;
  // BitBoard
  board_mask = uint64_t(0);
  bishop_mask.resize(2, 0);
  rook_mask.resize(2, 0);
  knight_mask.resize(2, 0);
  king_mask.resize(2, 0);
  pawn_mask.resize(2, 0);
  king_pos[0] = king_pos[1] = -1;

  buildBoard();
  boardMaskOccupancy();
  bishopMaskOccupancy();
  rookMaskOccupancy();
  knightMaskOccupancy();
  kingMaskOccupancy();
  pawnMaskOccupancy();

  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      int id = board[i][j];
      if(id == EMPTY || id == WK || id == BK) continue;
      gs.pieces_counter[id]++;

      if(id == WB) gs.pieces_counter[(i%2 + j%2)%2 == 0 ? WB_LIGHT : WB_DARK]++;
      else if(id == BB) gs.pieces_counter[(i%2 + j%2)%2 == 0 ? BB_LIGHT : BB_DARK]++;
    }
  }

  gs.zobristHash = ZOBRIST.compute(board, gs.castlingPreserved, gs.enPassant, isWhiteTurn());
  storeHashedBoard(gs.zobristHash);

  addState(gs);
}

void Game::boardMaskOccupancy() {
  for(int x=0;x<8;x++) {
    for(int y=0;y<8;y++) {
      if(board[x][y] != EMPTY) {
        int b = bitboard.grid2bit(x, y);
        board_mask |= bitboard.bit2mask(b);
      }
    }
  }
}

void Game::bishopMaskOccupancy() {
  for(int side=0;side<2;side++) {
    for(int x=0;x<8;x++) {
      for(int y=0;y<8;y++) {
        if(getColor(board[x][y]) == side && (isBishop(board[x][y]) || isQueen(board[x][y]))) {
          int b = bitboard.grid2bit(x, y);
          bishop_mask[side] |= bitboard.bit2mask(b);
        }
      }
    }
  }
}

void Game::rookMaskOccupancy() {
  for(int side=0;side<2;side++) {
    for(int x=0;x<8;x++) {
      for(int y=0;y<8;y++) {
        if(getColor(board[x][y]) == side && (isRook(board[x][y]) || isQueen(board[x][y]))) {
          int b = bitboard.grid2bit(x, y);
          rook_mask[side] |= bitboard.bit2mask(b);
        }
      }
    }
  }
}

void Game::knightMaskOccupancy() {
  for(int side=0;side<2;side++) {
    for(int x=0;x<8;x++) {
      for(int y=0;y<8;y++) {
        if(getColor(board[x][y]) == side && isKnight(board[x][y])) {
          int b = bitboard.grid2bit(x, y);
          knight_mask[side] |= bitboard.bit2mask(b);
        }
      }
    }
  }
}

void Game::kingMaskOccupancy() {
  for(int side=0;side<2;side++) {
    for(int x=0;x<8;x++) {
      for(int y=0;y<8;y++) {
        if(getColor(board[x][y]) == side && isKing(board[x][y])) {
          int b = bitboard.grid2bit(x, y);
          king_mask[side] |= bitboard.bit2mask(b);
        }
      }
    }
  }
}

void Game::pawnMaskOccupancy() {
  for(int side=0;side<2;side++) {
    for(int x=0;x<8;x++) {
      for(int y=0;y<8;y++) {
        if(getColor(board[x][y]) == side && isPawn(board[x][y])) {
          int b = bitboard.grid2bit(x, y);
          pawn_mask[side] |= bitboard.bit2mask(b);
        }
      }
    }
  }
}

void Game::setMaskPosition(int prev_piece, int new_piece, i2 position) {
  uint64_t mask = bitboard.bit2mask(bitboard.grid2bit(position.first, position.second));
  // board
  if(prev_piece != EMPTY) board_mask &= ~mask;
  if(new_piece != EMPTY) board_mask |= mask;

  // bishop mask
  if(isBishop(prev_piece) || isQueen(prev_piece)) bishop_mask[getColor(prev_piece)] &= ~mask;
  if(isBishop(new_piece) || isQueen(new_piece)) bishop_mask[getColor(new_piece)] |= mask;

  // rook mask
  if(isRook(prev_piece) || isQueen(prev_piece)) rook_mask[getColor(prev_piece)] &= ~mask;
  if(isRook(new_piece) || isQueen(new_piece)) rook_mask[getColor(new_piece)] |= mask;

  // knight mask
  if(isKnight(prev_piece)) knight_mask[getColor(prev_piece)] &= ~mask;
  if(isKnight(new_piece)) knight_mask[getColor(new_piece)] |= mask;

  // king mask
  if(isKing(prev_piece)) king_mask[getColor(prev_piece)] &= ~mask;
  if(isKing(new_piece)) king_mask[getColor(new_piece)] |= mask;

  // pawn mask
  if(isPawn(prev_piece)) pawn_mask[getColor(prev_piece)] &= ~mask;
  if(isPawn(new_piece)) pawn_mask[getColor(new_piece)] |= mask;
}

void Game::setBoard(int x, int y, int piece) {
  int prev_piece = getPositionInfo(x, y);
  board[x][y] = piece;
  setMaskPosition(prev_piece, piece, std::make_pair(x, y));
  if(!isKing(piece)) return;

  int b = bitboard.grid2bit(x, y);
  king_pos[getColor(piece)] = b;
}

GameState Game::getState() const {
  return gameState.back();
}

void Game::addState(GameState gs) {
  gameState.push_back(gs);
}

void Game::popState() {
  assert(gameState.size() > 0);
  gameState.pop_back();
}

bool Game::isWhiteTurn() const {
  return ((int)moves.size() % 2) == 0;
}

std::vector<std::vector<int>> Game::getBoard(int move_id) {
  if(move_id == -1) move_id = moves.size();

  std::vector<std::vector<int>> tmp = board;

  for(int i=(int)moves.size() - 1; i>=move_id; i--) {
    for(auto &c: moves[i]) {
      tmp[c.first.first][c.first.second] = c.second;
    }
  }

  return tmp;
}

int Game::storeHashedBoard(uint64_t hash) {
  Profiler::getInstance().start("Game.storeHashedBoard()");
  hashedBoardCounter[hash]++;
  int x = hashedBoardCounter[hash];
  Profiler::getInstance().stop("Game.storeHashedBoard()");
  return x;
}

vi4 Game::getMovesForPawn(i2 current_pos) {
  vi4 piece_moves;
  int piece = getPositionInfo(current_pos.first, current_pos.second);

  const GameState &gs = getState();

  int front_direction = (isWhiteTurn() ? -1 : 1);
  int initial_row = (7 + front_direction) % 7;

  // Left taking
  if(isValidMove(current_pos, {current_pos.first - 1, current_pos.second + front_direction})) {
    int info = getPositionInfo(current_pos.first - 1, current_pos.second + front_direction);
    if(info != EMPTY && getColor(info) != getColor(piece)) {
      piece_moves.push_back({current_pos, {current_pos.first - 1, current_pos.second + front_direction}});
    }
  }
  // Right taking
  if(isValidMove(current_pos, {current_pos.first + 1, current_pos.second + front_direction})) {
    int info = getPositionInfo(current_pos.first + 1, current_pos.second + front_direction);
    if(info != EMPTY && getColor(info) != getColor(piece)) {
      piece_moves.push_back({current_pos, {current_pos.first + 1, current_pos.second + front_direction}});
    }
  }
  // En passant
  if(gs.enPassant == std::make_pair(current_pos.first - 1, current_pos.second)
    || gs.enPassant == std::make_pair(current_pos.first + 1, current_pos.second)) {

    int attacker = getPositionInfo(current_pos.first, current_pos.second);
    int deffensor = getPositionInfo(gs.enPassant.first, gs.enPassant.second);

    setBoard(current_pos.first, current_pos.second, EMPTY);
    setBoard(gs.enPassant.first, gs.enPassant.second, EMPTY);
    setBoard(gs.enPassant.first, gs.enPassant.second + front_direction, attacker);

    if(!isOnCheck()) {
      piece_moves.push_back({current_pos, {gs.enPassant.first, gs.enPassant.second + front_direction}});
    }

    setBoard(current_pos.first, current_pos.second, attacker);
    setBoard(gs.enPassant.first, gs.enPassant.second, deffensor);
    setBoard(gs.enPassant.first, gs.enPassant.second + front_direction, EMPTY);
  }
  // Two moves
  if(current_pos.second == initial_row) {
    if(board[current_pos.first][current_pos.second + front_direction] == EMPTY
      && board[current_pos.first][current_pos.second + 2 * front_direction] == EMPTY) {

      if(isValidMove(current_pos, {current_pos.first, current_pos.second + 2 * front_direction})) {
        piece_moves.push_back({current_pos, {current_pos.first, current_pos.second + 2 * front_direction}});
      }
    }
  }
  // Single move
  if(getPositionInfo(current_pos.first, current_pos.second + front_direction) == EMPTY) {
    if(isValidMove(current_pos, {current_pos.first, current_pos.second + front_direction})) {
      piece_moves.push_back({current_pos, {current_pos.first, current_pos.second + front_direction}});
    }
  }

  return piece_moves;
}

vi4 Game::getMovesForRook(i2 current_pos) {
  vi4 piece_moves;

  int dl2[] = {-1, 0, 1, 0};
  int dc2[] = {0, -1, 0, 1};
  for(int j=0;j<4;j++) {
    i2 new_pos = current_pos;
    for(int k=0;k<8;k++) {
      new_pos.first += dl2[j];
      new_pos.second += dc2[j];

      if(isValidMove(current_pos, new_pos)) {
        piece_moves.push_back({current_pos, new_pos});
      }

      if(getPositionInfo(new_pos.first, new_pos.second) != EMPTY) break;
    }
  }

  return piece_moves;
}

vi4 Game::getMovesForKnight(i2 current_pos) {
  vi4 piece_moves;

  int dl[] = {-2, -2, -1, 1, 2, 2, -1, 1};
  int dc[] = {-1, 1, 2, 2, -1, 1, -2, -2};
  for(int j=0;j<8;j++) {
    i2 new_pos = current_pos;
    new_pos.first += dl[j];
    new_pos.second += dc[j];

    if(isValidMove(current_pos, new_pos)) {
      piece_moves.push_back({current_pos, new_pos});
    }
  }

  return piece_moves;
}

vi4 Game::getMovesForBishop(i2 current_pos) {
  vi4 piece_moves;

  int dl3[] = {-1, -1, 1, 1};
  int dc3[] = {-1, 1, 1, -1};
  for(int j=0;j<4;j++) {
    i2 new_pos = current_pos;
    for(int k=0;k<8;k++) {
      new_pos.first += dl3[j];
      new_pos.second += dc3[j];

      if(isValidMove(current_pos, new_pos)) {
        piece_moves.push_back({current_pos, new_pos});
      }

      if(getPositionInfo(new_pos.first, new_pos.second) != EMPTY) break;
    }
  }

  return piece_moves;
}

vi4 Game::getMovesForQueen(i2 pos) {
  vi4 piece_moves = getMovesForRook(pos);
  vi4 b_moves = getMovesForBishop(pos);

  piece_moves.insert(piece_moves.end(), b_moves.begin(), b_moves.end());
  return piece_moves;
}

vi4 Game::getMovesForKing(i2 current_pos) {
  vi4 piece_moves;
  const GameState &gs = getState();

  int dl1[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc1[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  for(int j=0;j<8;j++) {
    i2 new_pos = current_pos;
    new_pos.first += dl1[j];
    new_pos.second += dc1[j];

    if(isValidMove(current_pos, new_pos)) {
      piece_moves.push_back({current_pos, new_pos});
    }
  }
  // White Castling: left side
  if(isWhiteTurn() && gs.isCastlingPreserved(0) && getPositionInfo(0, 7) == WR
    && getPositionInfo(1, 7) == EMPTY && getPositionInfo(2, 7) == EMPTY && getPositionInfo(3, 7) == EMPTY && !isOnCheck()) {

    if(isValidMove({4, 7}, {3, 7}) && isValidMove({4, 7}, {2, 7})) {
      piece_moves.push_back({{4, 7}, {2, 7}});
    } 
  }
  // Black Castling: left side
  if(!isWhiteTurn() && gs.isCastlingPreserved(2) && getPositionInfo(0, 0) == BR
    && getPositionInfo(1, 0) == EMPTY && getPositionInfo(2, 0) == EMPTY && getPositionInfo(3, 0) == EMPTY &&  !isOnCheck()) {

    if(isValidMove({4, 0}, {3, 0}) && isValidMove({4, 0}, {2, 0})) {
      piece_moves.push_back({{4, 0}, {2, 0}});
    } 
  }
  if(isWhiteTurn() && gs.isCastlingPreserved(1) && getPositionInfo(7, 7) == WR
    && getPositionInfo(5, 7) == EMPTY && getPositionInfo(6, 7) == EMPTY && !isOnCheck()) {

    if(isValidMove({4, 7}, {5, 7}) && isValidMove({4, 7}, {6, 7})) {
      piece_moves.push_back({{4, 7}, {6, 7}});
    }
  }
  // Black Castling: right side
  if(!isWhiteTurn() && gs.isCastlingPreserved(3) && getPositionInfo(7, 0) == BR
    && getPositionInfo(5, 0) == EMPTY && getPositionInfo(6, 0) == EMPTY && !isOnCheck()) {

    if(isValidMove({4, 0}, {5, 0}) && isValidMove({4, 0}, {6, 0})) {
      piece_moves.push_back({{4, 0}, {6, 0}});
    }
  }

  return piece_moves;
}

vi4 Game::getMovesFor(i2 pos) {
  int piece = getPositionInfo(pos.first, pos.second);
  if(piece == OUT || piece == EMPTY) return vi4();
  int curr_color = !isWhiteTurn();
  if(curr_color != getColor(piece)) return vi4();

  if(isPawn(piece)) return getMovesForPawn(pos);
  else if(isRook(piece)) return getMovesForRook(pos);
  else if(isKnight(piece)) return getMovesForKnight(pos);
  else if(isBishop(piece)) return getMovesForBishop(pos);
  else if(isQueen(piece)) return getMovesForQueen(pos);
  else return getMovesForKing(pos);
}

vi3 Game::getSpecialCells(i2 cell) {
  vi3 cells;
  if(moves.size() > 0) {
    for(auto &m: moves.back()) {
      cells.push_back({m.first, 3});
    }
  }

  if(isDraw()) {
    cells.push_back({getKingPos(true), -1});
    cells.push_back({getKingPos(false), -1});
  } else if(isCheckMate()) {
    cells.push_back({getKingPos(isWhiteTurn()), 1});
  } else {
    vi4 movesForCell = getMovesFor(cell);

    for(auto &m_cell: movesForCell) {
      cells.push_back({m_cell.second, 0});
    }
    if(movesForCell.size() > 0) cells.push_back({cell, 2});
  }
  return cells;
}

i2 Game::getKingPos(bool white) {
  int b = king_pos[!white];
  assert(b != -1);
  return bitboard.bit2grid(b);
}

bool Game::isDraw() const {
  return getState().gameStatus == DRAW;
}

bool Game::isCheckMate() const {
  return getState().gameStatus == CHECKMATE;
}

void Game::buildBoard() {
  for(int i=0;i<8;i++) {
    std::vector<int> row;
    for(int j=0;j<8;j++) {
      row.push_back(EMPTY);
    }
    board.push_back(row);
  }

  setBoard(0, 0, BR);
  setBoard(1, 0, BN);
  setBoard(2, 0, BB);
  setBoard(3, 0, BQ);
  setBoard(4, 0, BK);
  setBoard(5, 0, BB);
  setBoard(6, 0, BN);
  setBoard(7, 0, BR);
  setBoard(0, 7, WR);
  setBoard(1, 7, WN);
  setBoard(2, 7, WB);
  setBoard(3, 7, WQ);
  setBoard(4, 7, WK);
  setBoard(5, 7, WB);
  setBoard(6, 7, WN);
  setBoard(7, 7, WR);

  for(int i=0;i<8;i++) {
    setBoard(i, 1, BP);
    setBoard(i, 6, WP);
  }
}

int Game::getPositionInfo(int x, int y) const {
  if(x < 0 || x > 7 || y < 0 || y > 7) return OUT;
  return board[x][y];
}

bool Game::isOnCheck() {
  i2 k_pos = getKingPos(isWhiteTurn());
  int king_x = k_pos.first;
  int king_y = k_pos.second;

  int black = isWhiteTurn();
  int cell = bitboard.grid2bit(king_x, king_y);
  uint64_t mask;
  bool ret = false;

  // Checked by a Pawn
  if(isWhiteTurn()) {
    if(getPositionInfo(king_x-1, king_y-1) == BP || getPositionInfo(king_x+1, king_y-1) == BP) ret = true;
  } else {
    if(getPositionInfo(king_x-1, king_y+1) == WP || getPositionInfo(king_x+1, king_y+1) == WP) ret = true;
  }

  // Checked by a King
  mask = bitboard.king(cell);
  uint64_t k_mask = king_mask[black];
  ret |= (mask&k_mask);

  // Checked by a Knight
  mask = bitboard.knight(cell);
  uint64_t n_mask = knight_mask[black];
  ret |= (mask&n_mask);

  // Checked by a Bishop / Queen
  mask = bitboard.bishop(cell, board_mask);
  uint64_t b_mask = bishop_mask[black];
  ret |= (mask&b_mask);

  // Checked by a Rook / Queen
  mask = bitboard.rook(cell, board_mask);
  uint64_t r_mask = rook_mask[black];
  ret |= (mask&r_mask);

  return ret;
}

// Same reverse-ray-casting trick as isOnCheck() (compute the attack pattern
// as if a piece of each type stood on `pos`, then check whether an actual
// attacker of that type/color occupies one of those squares), generalized
// to an arbitrary square and color instead of being hardcoded to the king
// in check.
bool Game::isAttackedBy(i2 pos, int attackerColor) const {
  int x = pos.first, y = pos.second;
  int cell = bitboard.grid2bit(x, y);

  if(attackerColor == 0) { // white pawns attack from one rank below (higher row index)
    if(getPositionInfo(x-1, y+1) == WP || getPositionInfo(x+1, y+1) == WP) return true;
  } else { // black pawns attack from one rank above (lower row index)
    if(getPositionInfo(x-1, y-1) == BP || getPositionInfo(x+1, y-1) == BP) return true;
  }

  if(bitboard.knight(cell) & knight_mask[attackerColor]) return true;
  if(bitboard.king(cell) & king_mask[attackerColor]) return true;
  if(bitboard.bishop(cell, board_mask) & bishop_mask[attackerColor]) return true;
  if(bitboard.rook(cell, board_mask) & rook_mask[attackerColor]) return true;

  return false;
}

// Static "hanging piece" penalty: any piece that's attacked by the enemy
// and has no defender of its own color loses its full value from whichever
// side owns it -- this is what a fixed-depth search on its own can miss at
// the horizon (e.g. a piece sacrificed on the search's last ply, with the
// recapture sitting one ply beyond it, gets evaluated as if the sac were
// free). Deliberately simple: it doesn't weigh attacker vs defender value
// (an actually-defended piece is treated as fully safe), trading precision
// for being O(64) with no recursion.
double Game::hangingPiecesScore() const {
  double sc = 0.0;

  for(int x=0;x<8;x++) {
    for(int y=0;y<8;y++) {
      int piece = board[x][y];
      if(piece == EMPTY) continue;

      int color = getColor(piece);
      int opponent = 1 - color;
      if(!isAttackedBy({x, y}, opponent)) continue;
      if(isAttackedBy({x, y}, color)) continue; // defended: treated as safe

      sc -= evaluatePiece(piece); // signed value handles white/black direction
    }
  }

  return sc;
}

bool Game::isValidMove(i2 curr_pos, i2 new_pos) {
  int current_pos_before = getPositionInfo(curr_pos.first, curr_pos.second);
  int new_pos_before = getPositionInfo(new_pos.first, new_pos.second);

  if(new_pos_before == OUT) return false;
  if(isWhiteTurn() && isWhite(new_pos_before)) return false;
  if(!isWhiteTurn() && isBlack(new_pos_before)) return false;
  if(isKing(new_pos_before)) return false;

  // Move the piece
  setBoard(curr_pos.first, curr_pos.second, EMPTY);
  setBoard(new_pos.first, new_pos.second, current_pos_before);

  bool isValid = !isOnCheck();

  // Rollback board
  setBoard(curr_pos.first, curr_pos.second, current_pos_before);
  setBoard(new_pos.first, new_pos.second, new_pos_before);
  return isValid;
}

vi4 Game::genNextMoves() {
  vi4 nextMoves;

  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      vi4 new_moves;
      new_moves = getMovesFor(std::make_pair(i, j));
      nextMoves.insert(nextMoves.end(), new_moves.begin(), new_moves.end());
    }
  }

  return nextMoves;
}

double Game::evaluatePiece(int piece) const {
  if(piece == EMPTY) return 0.0;
  int mult = (isWhite(piece) ? 1.0 : -1.0);

  double value = 0.0;

  if(isRook(piece)) value = 5.0;
  else if(isKnight(piece)) value = 3.0;
  else if(isBishop(piece)) value = 3.0;
  else if(isQueen(piece)) value = 9.0;
  else if(isPawn(piece)) value = 1.0;

  return mult * value;
}

void Game::executeMove(vi3 &move, GameState &gs) {
  vi3 rollback;
  double score = 0.0;

  for(auto &m: move) {
    int curr_piece = getPositionInfo(m.first.first, m.first.second);
    rollback.push_back({m.first, curr_piece});

    ZOBRIST.togglePiece(gs.zobristHash, curr_piece, m.first.first, m.first.second);
    ZOBRIST.togglePiece(gs.zobristHash, m.second, m.first.first, m.first.second);

    setBoard(m.first.first, m.first.second, m.second);

    score -= evaluatePiece(curr_piece);
    score += evaluatePiece(m.second);

    std::vector<std::pair<int, int>> tmp;
    tmp.push_back({curr_piece, -1});
    tmp.push_back({m.second, 1});

    for(auto &t: tmp) {
      int id = t.first;
      if(id == EMPTY || id == BK || id == WK) continue;

      gs.pieces_counter[id] += t.second;

      if(id == WB) gs.pieces_counter[(m.first.first%2 + m.first.second%2)%2 == 0 ? WB_LIGHT : WB_DARK] += t.second;
      else if(id == BB) gs.pieces_counter[(m.first.first%2 + m.first.second%2)%2 == 0 ? BB_LIGHT : BB_DARK] += t.second;
    }
  }

  moves.push_back(rollback);
  gs.piecesScoring += score;
}

void Game::undoAction() {
  Profiler::getInstance().start("Game::undoAction");

  hashedBoardCounter[getState().zobristHash]--;
  gameState.pop_back();

  auto &undo_move = moves.back();
  for(auto &m: undo_move) {
    setBoard(m.first.first, m.first.second, m.second);
  }
  moves.pop_back();
  Profiler::getInstance().stop("Game::undoAction");
}

void Game::doAction(i2 current_pos, i2 new_pos, int choose) {
  Profiler::getInstance().start("Game::doAction");
  const GameState curr_gs = getState();
  GameState new_gs = curr_gs;
  new_gs.enPassant = {-1, -1};

  int piece = getPositionInfo(current_pos.first, current_pos.second);
  vi3 current_move;

  if(isPawn(piece) && getPositionInfo(new_pos.first, new_pos.second) == EMPTY && current_pos.first != new_pos.first) {
    // Action: En passant
    current_move.push_back({{current_pos.first, current_pos.second}, EMPTY});
    current_move.push_back({{new_pos.first, new_pos.second}, piece});
    current_move.push_back({{curr_gs.enPassant.first, curr_gs.enPassant.second}, EMPTY});

  } else if(isKing(piece) && int(std::abs(current_pos.first - new_pos.first)) == 2) {
    // Action: Castling
    int row = current_pos.second;
    if(new_pos.first == 2) {
      int rook = board[0][row];

      current_move.push_back({{0, row}, EMPTY});
      current_move.push_back({{2, row}, piece});
      current_move.push_back({{3, row}, rook});
      current_move.push_back({{4, row}, EMPTY});

    } else {
      int rook = board[7][row];

      current_move.push_back({{7, row}, EMPTY});
      current_move.push_back({{6, row}, piece});
      current_move.push_back({{5, row}, rook});
      current_move.push_back({{4, row}, EMPTY});

    }

    if(isWhiteTurn()) {
      new_gs.touch(0);
      new_gs.touch(1);
      new_gs.doCastling(0);
    } else {
      new_gs.touch(2);
      new_gs.touch(3);
      new_gs.doCastling(1);
    }
  } else if(isPawn(piece) && int(std::abs(current_pos.second - new_pos.second)) == 2) {
    // Action: Two moves
    new_gs.enPassant = new_pos;

    current_move.push_back({{current_pos.first, current_pos.second}, EMPTY});
    current_move.push_back({{new_pos.first, new_pos.second}, piece});

  } else if(isPawn(piece) && (new_pos.second == 0 || new_pos.second == 7)) {
    // Action: Promotion
    assert(choose != -1);
    int promotedPiece = -1;
    if(choose == 0) promotedPiece = (isWhiteTurn() ? WQ : BQ);
    else if(choose == 1) promotedPiece = (isWhiteTurn() ? WR : BR);
    else if(choose == 2) promotedPiece = (isWhiteTurn() ? WN : BN);
    else if(choose == 3) promotedPiece = (isWhiteTurn() ? WB : BB);
    else assert(false);

    current_move.push_back({{current_pos.first, current_pos.second}, EMPTY});
    current_move.push_back({{new_pos.first, new_pos.second}, promotedPiece});

  } else {
    // Any other move
    current_move.push_back({{current_pos.first, current_pos.second}, EMPTY});
    current_move.push_back({{new_pos.first, new_pos.second}, piece});

  }
  executeMove(current_move, new_gs);

  if(piece == WK) new_gs.touch(0), new_gs.touch(1);
  if(piece == BK) new_gs.touch(2), new_gs.touch(3);
  if(piece == WR && current_pos.first == 0) new_gs.touch(0);
  if(piece == WR && current_pos.first == 7) new_gs.touch(1);
  if(piece == BR && current_pos.first == 0) new_gs.touch(2);
  if(piece == BR && current_pos.first == 7) new_gs.touch(3);

  // Side-to-move always flips; castling rights and en passant have only
  // just reached their final value for this move (touch() above can still
  // change castlingPreserved), so their hash contribution is finished here
  // instead of incrementally inside executeMove().
  ZOBRIST.toggleSideToMove(new_gs.zobristHash);
  ZOBRIST.updateCastling(new_gs.zobristHash, curr_gs.castlingPreserved, new_gs.castlingPreserved);
  ZOBRIST.updateEnPassant(new_gs.zobristHash, curr_gs.enPassant, new_gs.enPassant);

#ifdef ZOBRIST_VERIFY
  assert(new_gs.zobristHash == ZOBRIST.compute(board, new_gs.castlingPreserved, new_gs.enPassant, isWhiteTurn()));
#endif

  new_gs.repetition = storeHashedBoard(new_gs.zobristHash) == 3;

  addState(new_gs); // hasAnyMove uses GameState
  new_gs.hasMoves = hasAnyMove();
  popState();

  if(drawConditions(new_gs)) {
    new_gs.gameStatus = DRAW;
  }
  if(!new_gs.hasMoves && isOnCheck()) {
    new_gs.gameStatus = CHECKMATE;
  }

  addState(new_gs);
  Profiler::getInstance().stop("Game::doAction");
}

bool Game::hasMoveFor(i2 pos) {
  return getMovesFor(pos).size() > 0;
}

bool Game::hasAnyMove() {
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      if(hasMoveFor({i, j})) return true;
    }
  }
  return false;
}

bool Game::isAvailable(i2 curr_pos, i2 new_pos) {
  vi4 curr_pos_moves = getMovesFor(curr_pos);

  for(auto &m: curr_pos_moves) {
    if(m.second == new_pos) return true;
  }

  return false;
}

bool Game::isPawnPromotion(i2 curr_pos, i2 new_pos) {
  int promotion_y = (isWhiteTurn() ? 0 : 7);
  return (isPawn(getPositionInfo(curr_pos.first, curr_pos.second)) && new_pos.second == promotion_y);
}

bool Game::drawConditions(const GameState &gs) const {
  // Repetition
  if(gs.repetition) return true;

  // Stalemate
  if(!gs.hasMoves) return true;
  // Insufficient mating material
  bool isInsufficient = true;
  int total_pieces = 0;
  // indices 0..11 are the real WR..BP material counts; the WB_LIGHT/WB_DARK/
  // BB_LIGHT/BB_DARK slots are just a color breakdown of WB/BB and would
  // double-count bishops if included here
  for(int i=0;i<12;i++) {
    total_pieces += gs.pieces_counter[i];
  }

  if(total_pieces > 2) isInsufficient = false;
  else if(total_pieces == 2) {
    // a rook/queen/pawn among the two remaining pieces is already enough to
    // force mate on its own (see the total_pieces == 1 case below)
    bool hasHeavyPiece = gs.pieces_counter[WR] || gs.pieces_counter[WQ] || gs.pieces_counter[WP]
                       || gs.pieces_counter[BR] || gs.pieces_counter[BQ] || gs.pieces_counter[BP];

    if(hasHeavyPiece) {
      isInsufficient = false;
    } else {
      int whiteBishops = gs.pieces_counter[WB];
      int blackBishops = gs.pieces_counter[BB];

      if(whiteBishops == 2) {
        // sufficient only if the pair covers both square colors (forced mate exists)
        isInsufficient = (gs.pieces_counter[WB_LIGHT] == 0 || gs.pieces_counter[WB_DARK] == 0);
      } else if(blackBishops == 2) {
        isInsufficient = (gs.pieces_counter[BB_LIGHT] == 0 || gs.pieces_counter[BB_DARK] == 0);
      } else {
        // any other split of the two minor pieces -- one per side, two
        // knights on the same side, or knight+bishop on the same side --
        // can't force mate against a lone king. This ignores the rare and
        // hard-to-execute bishop+knight helpmate as an accepted simplification.
        isInsufficient = true;
      }
    }
  } else if(total_pieces == 1) {
    const std::vector<int> pos = {WR, WQ, WP, BR, BQ, BP};
    for(auto &p: pos) {
      if(gs.pieces_counter[p] > 0) isInsufficient = false;
    }
  }

  if(isInsufficient) return true;
  // All checks have been passed
  return false;
}

int Game::getTotalMoves() const {
  return moves.size();
}

double Game::positionalScoring() const {
  double sc = 0.0;
  double signal = 1.0;
  double bonus_factor = 0.1;

  // Bishop
  for(uint64_t mask: bishop_mask) {
    uint64_t bmask = mask;
    double bishop_score = 3.0;
    double bishop_reducer = 8+7;

    while(bmask) {
      uint64_t active_bit = (bmask & -bmask);
      double free_positions = __builtin_popcountll(
        bitboard.bishop(
          63 - __builtin_clzll(active_bit),
          board_mask
        )
      );

      sc += (signal * (free_positions/bishop_reducer) * bishop_score) * bonus_factor;
      bmask ^= active_bit;
    }
    signal *= -1.0;
  }

  // Rook
  signal = 1.0;
  for(uint64_t mask: rook_mask) {
    uint64_t bmask = mask;
    double rook_score = 5.0;
    double rook_reducer = 8+8;

    while(bmask) {
      uint64_t active_bit = (bmask & -bmask);
      double free_positions = __builtin_popcountll(
        bitboard.rook(
          63 - __builtin_clzll(active_bit),
          board_mask
        )
      );

      sc += (signal * (free_positions/rook_reducer) * rook_score) * bonus_factor;
      bmask ^= active_bit;
    }
    signal *= -1.0;
  }

  // Knight
  signal = 1.0;
  for(uint64_t mask: knight_mask) {
    uint64_t bmask = mask;
    double knight_score = 3.0;
    double knight_reducer = 8;

    while(bmask) {
      uint64_t active_bit = (bmask & -bmask);
      double free_positions = __builtin_popcountll(
        bitboard.knight(
          63 - __builtin_clzll(active_bit)
        )
      );

      sc += (signal * (free_positions/knight_reducer) * knight_score) * bonus_factor;
      bmask ^= active_bit;
    }
    signal *= -1.0;
  }

  // Pawn Structure - White
  uint64_t bmask = pawn_mask[0];
  uint64_t defensor_mask = (bmask&(~FILEA))<<7;
  defensor_mask |= (bmask&(~FILEH))<<9;
  sc += __builtin_popcountll(defensor_mask) * bonus_factor;

  // Pawn Structure - Black
  bmask = pawn_mask[1];
  defensor_mask = (bmask&(~FILEH))>>7;
  defensor_mask |= (bmask&(~FILEA))>>9;
  sc -= __builtin_popcountll(defensor_mask) * bonus_factor;

  uint64_t rank_mask = RANK1<<8;
  double bonus_id = 0;
  while((rank_mask&RANK8) == 0) {
    uint64_t wp_mask = pawn_mask[0];
    uint64_t bp_mask = pawn_mask[1];

    sc += __builtin_popcountll(wp_mask&rank_mask) * bonus_factor * bonus_id;
    sc -= __builtin_popcountll(bp_mask&rank_mask) * bonus_factor * (5.0 - bonus_id);

    rank_mask <<= 8;
    bonus_id += 1.0;
  }

  return sc;
}

double Game::getScore() const {
  const GameState &gs = getState();

  if(gs.gameStatus == DRAW) return 0.0;
  else if(gs.gameStatus == CHECKMATE) {
    if(isWhiteTurn()) return -1000.0;
    else return 1000.0;
  }

  return gs.piecesScoring + gs.scoringHeuristic() + positionalScoring() + hangingPiecesScore();
}

double Game::getCellScore(int x, int y) const {
  int info = getPositionInfo(x, y);
  assert(info != OUT);

  return evaluatePiece(info);
}

void Game::debugger() {
  genNextMoves();
}
