#include <Game.hpp>
#include <chrono>
#include <iomanip>
#include <Bitboard.hpp>

Game::Game() {
  GameState gs;
  gs.gameStatus = ALIVE;
  gs.enPassant = {-1, -1};
  gs.castlingPreserved = 0;
  gs.piecesScoring = 0.0;
  gs.moves_white = 0;
  gs.moves_black = 0;
  gs.repetition = false;
  gs.pieces_counter.resize(12, 0);
  gs.castled = 0;
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
      if(id == WB || id == BB) {
        id += (i%2 + j%2)%2;
      }
      gs.pieces_counter[id]++;
    }
  }

  addState(gs);
  genNextMoves(gs);
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

void Game::setMaskPosition(int prev_piece, int new_piece, pii position) {
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
  int prev_piece = board[x][y];
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

std::string Game::getBoardHash() {
  std::string hsh = "";
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      if(board[i][j] == EMPTY) hsh += "/x";
      else hsh += "/" + std::to_string(board[i][j]);
    }
  }

  return hsh;
}

int Game::storeHashedBoard() {
  const std::string &hsh = getBoardHash();
  hashedBoardCounter[hsh]++;
  return hashedBoardCounter[hsh];
}

std::vector<std::pair<pii, int>> Game::getSpecialCells(pii cell) {
  std::vector<std::pair<pii, int>> cells;
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
    for(int i=0;i<nextMoves.size();i++) {
      if(nextMoves[i].first == cell) cells.push_back({nextMoves[i].second, 0});
    }
    if(hasMoveFor(cell)) cells.push_back({cell, 2});
  }
  return cells;
}

pii Game::getKingPos(bool white) {
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
  storeHashedBoard();
}

int Game::getPositionInfo(int x, int y) const {
  if(x < 0 || x > 7 || y < 0 || y > 7) return OUT;
  return board[x][y];
}

bool Game::isOnCheck() {
  std::clock_t t = std::clock();
  pii k_pos = getKingPos(isWhiteTurn());
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
  

  t = (std::clock() - t);
  elapsed_sec["isOnCheck"] += ((double)t/CLOCKS_PER_SEC) * 1000.0;
  called_counter["isOnCheck"]++;
  return ret;
}

bool Game::isValidMove(pii curr_pos, pii new_pos) {
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

void Game::genNextMoves(const GameState &gs) {
  std::clock_t t = std::clock();
  nextMoves.clear();

  std::vector<std::pair<int, pii>> setup;
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      if(board[i][j] == EMPTY) continue;
      setup.push_back({board[i][j], {i, j}});
    }
  }

  for(int i=0;i<setup.size();i++) {
    if(isWhiteTurn() && isBlack(setup[i].first)) continue;
    if(!isWhiteTurn() && isWhite(setup[i].first)) continue;

    pii current_pos = setup[i].second;

    if(isKnight(setup[i].first)) {
      // Knight moves
      int dl[] = {-2, -2, -1, 1, 2, 2, -1, 1};
      int dc[] = {-1, 1, 2, 2, -1, 1, -2, -2};
      for(int j=0;j<8;j++) {
        pii new_pos = setup[i].second;
        new_pos.first += dl[j];
        new_pos.second += dc[j];

        if(isValidMove(current_pos, new_pos)) {
          nextMoves.push_back({current_pos, new_pos});
        }
      }
    }
    if(isKing(setup[i].first)) {
      // King moves
      int dl1[] = {-1, -1, -1, 0, 0, 1, 1, 1};
      int dc1[] = {-1, 0, 1, -1, 1, -1, 0, 1};
      for(int j=0;j<8;j++) {
        pii new_pos = setup[i].second;
        new_pos.first += dl1[j];
        new_pos.second += dc1[j];

        if(isValidMove(current_pos, new_pos)) {
          nextMoves.push_back({current_pos, new_pos});
        }
      }
      // White Castling: left side
      if(isWhiteTurn() && gs.isCastlingPreserved(0) && getPositionInfo(0, 7) == WR
        && getPositionInfo(1, 7) == EMPTY && getPositionInfo(2, 7) == EMPTY && getPositionInfo(3, 7) == EMPTY && !isOnCheck()) {

        if(isValidMove({4, 7}, {3, 7}) && isValidMove({4, 7}, {2, 7})) {
          nextMoves.push_back({{4, 7}, {2, 7}});
        } 
      }
      // Black Castling: left side
      if(!isWhiteTurn() && gs.isCastlingPreserved(2) && getPositionInfo(0, 0) == BR
        && getPositionInfo(1, 0) == EMPTY && getPositionInfo(2, 0) == EMPTY && getPositionInfo(3, 0) == EMPTY &&  !isOnCheck()) {

        if(isValidMove({4, 0}, {3, 0}) && isValidMove({4, 0}, {2, 0})) {
          nextMoves.push_back({{4, 0}, {2, 0}});
        } 
      }
      if(isWhiteTurn() && gs.isCastlingPreserved(1) && getPositionInfo(7, 7) == WR
        && getPositionInfo(5, 7) == EMPTY && getPositionInfo(6, 7) == EMPTY && !isOnCheck()) {

        if(isValidMove({4, 7}, {5, 7}) && isValidMove({4, 7}, {6, 7})) {
          nextMoves.push_back({{4, 7}, {6, 7}});
        }
      }
      // Black Castling: right side
      if(!isWhiteTurn() && gs.isCastlingPreserved(3) && getPositionInfo(7, 0) == BR
        && getPositionInfo(5, 0) == EMPTY && getPositionInfo(6, 0) == EMPTY && !isOnCheck()) {

        if(isValidMove({4, 0}, {5, 0}) && isValidMove({4, 0}, {6, 0})) {
          nextMoves.push_back({{4, 0}, {6, 0}});
        }
      }
    }
    if(isRook(setup[i].first) || isQueen(setup[i].first)) {
      // Rook & Queen moves
      int dl2[] = {-1, 0, 1, 0};
      int dc2[] = {0, -1, 0, 1};
      for(int j=0;j<4;j++) {
        pii new_pos = current_pos;
        for(int k=0;k<8;k++) {
          new_pos.first += dl2[j];
          new_pos.second += dc2[j];

          if(isValidMove(current_pos, new_pos)) {
            nextMoves.push_back({current_pos, new_pos});
          }

          if(getPositionInfo(new_pos.first, new_pos.second) != EMPTY) break;
        }
      }
    }
    if(isBishop(setup[i].first) || isQueen(setup[i].first)) {
      // Bishop & Queen moves
      int dl3[] = {-1, -1, 1, 1};
      int dc3[] = {-1, 1, 1, -1};
      for(int j=0;j<4;j++) {
        pii new_pos = current_pos;
        for(int k=0;k<8;k++) {
          new_pos.first += dl3[j];
          new_pos.second += dc3[j];

          if(isValidMove(current_pos, new_pos)) {
            nextMoves.push_back({current_pos, new_pos});
          }

          if(getPositionInfo(new_pos.first, new_pos.second) != EMPTY) break;
        }
      }
    }
    if(isPawn(setup[i].first)) {
      int front_direction = (isWhiteTurn() ? -1 : 1);
      int initial_row = (7 + front_direction) % 7;

      // Left taking
      if(isValidMove(current_pos, {current_pos.first - 1, current_pos.second + front_direction})) {
        int info = getPositionInfo(current_pos.first - 1, current_pos.second + front_direction);
        if(info != EMPTY && getColor(info) != getColor(setup[i].first)) {
          nextMoves.push_back({current_pos, {current_pos.first - 1, current_pos.second + front_direction}});
        }
      }
      // Right taking
      if(isValidMove(current_pos, {current_pos.first + 1, current_pos.second + front_direction})) {
        int info = getPositionInfo(current_pos.first + 1, current_pos.second + front_direction);
        if(info != EMPTY && getColor(info) != getColor(setup[i].first)) {
          nextMoves.push_back({current_pos, {current_pos.first + 1, current_pos.second + front_direction}});
        }
      }
      // En passant
      if(gs.enPassant == std::make_pair(current_pos.first - 1, current_pos.second)
        || gs.enPassant == std::make_pair(current_pos.first + 1, current_pos.second)) {

        int attacker = board[current_pos.first][current_pos.second];
        int deffensor = board[gs.enPassant.first][gs.enPassant.second];

        setBoard(current_pos.first, current_pos.second, EMPTY);
        setBoard(gs.enPassant.first, gs.enPassant.second, EMPTY);
        setBoard(gs.enPassant.first, gs.enPassant.second + front_direction, attacker);

        if(!isOnCheck()) {
          nextMoves.push_back({current_pos, {gs.enPassant.first, gs.enPassant.second + front_direction}});
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
            nextMoves.push_back({current_pos, {current_pos.first, current_pos.second + 2 * front_direction}});
          }
        }
      }
      // Single move
      if(getPositionInfo(current_pos.first, current_pos.second + front_direction) == EMPTY) {
        if(isValidMove(current_pos, {current_pos.first, current_pos.second + front_direction})) {
          nextMoves.push_back({current_pos, {current_pos.first, current_pos.second + front_direction}});
        }
      }
    }
  }
  t = (std::clock() - t);
  elapsed_sec["genNextMoves"] += ((double)t/CLOCKS_PER_SEC) * 1000.0;
  called_counter["genNextMoves"]++;
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

void Game::executeMove(std::vector<std::pair<pii, int>> &move, GameState &gs) {
  std::clock_t t = std::clock();
  std::vector<std::pair<pii, int>> rollback;
  double score = 0.0;

  for(auto &m: move) {
    int curr_piece = board[m.first.first][m.first.second];
    rollback.push_back({m.first, curr_piece});
    setBoard(m.first.first, m.first.second, m.second);

    score -= evaluatePiece(curr_piece);
    score += evaluatePiece(m.second);

    std::vector<std::pair<int, int>> tmp;
    tmp.push_back({curr_piece, -1});
    tmp.push_back({m.second, 1});

    for(auto &t: tmp) {
      int id = t.first;
      if(id == WB || id == BB) {
        id += (m.first.first%2 + m.first.second%2)%2;
      }
      if(id == EMPTY || id == BK || id == WK) continue;

      gs.pieces_counter[id] += t.second;
    }
  }

  moves.push_back(rollback);
  gs.piecesScoring += score;

  t = (std::clock() - t);
  elapsed_sec["executeMove"] += ((double)t/CLOCKS_PER_SEC) * 1000.0;
  called_counter["executeMove"]++;
}

void Game::undoAction() {
  gameState.pop_back();

  hashedBoardCounter[getBoardHash()]--;

  auto &undo_move = moves.back();
  for(auto &m: undo_move) {
    setBoard(m.first.first, m.first.second, m.second);
  }
  moves.pop_back();

  // TODO: Remove genNextMoves here
  genNextMoves(gameState.back());
}

void Game::doAction(pii current_pos, pii new_pos, int choose) {
  std::clock_t t = std::clock();

  const GameState curr_gs = getState();
  GameState new_gs = curr_gs;
  new_gs.enPassant = {-1, -1};

  int piece = board[current_pos.first][current_pos.second];
  std::vector<std::pair<pii, int>> current_move;

  if(isPawn(piece) && board[new_pos.first][new_pos.second] == EMPTY && current_pos.first != new_pos.first) {
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
  new_gs.repetition = storeHashedBoard() == 3;

  if(piece == WK) new_gs.touch(0), new_gs.touch(1);
  if(piece == BK) new_gs.touch(2), new_gs.touch(3);
  if(piece == WR && current_pos.first == 0) new_gs.touch(0);
  if(piece == WR && current_pos.first == 7) new_gs.touch(1);
  if(piece == BR && current_pos.first == 0) new_gs.touch(2);
  if(piece == BR && current_pos.first == 7) new_gs.touch(3);

  genNextMoves(new_gs);

  if(isWhiteTurn()) new_gs.moves_white = nextMoves.size();
  else new_gs.moves_black = nextMoves.size();

  if(drawConditions(new_gs)) {
    new_gs.gameStatus = DRAW;
  }
  if(nextMoves.size() == 0 && isOnCheck()) {
    new_gs.gameStatus = CHECKMATE;
  }

  addState(new_gs);

  t = (std::clock() - t);
  elapsed_sec["doAction"] += ((double)t/CLOCKS_PER_SEC) * 1000.0;
  called_counter["doAction"]++;
}

bool Game::hasMoveFor(pii pos) {
  for(int i=0;i<nextMoves.size();i++) {
    if(nextMoves[i].first == pos) return true;
  }
  return false;
}

bool Game::isAvailable(pii curr_pos, pii new_pos) {
  std::clock_t t = std::clock();
  for(int i=0;i<nextMoves.size();i++) {
    if(nextMoves[i].first == curr_pos && nextMoves[i].second == new_pos) return true;
  }
  t = (std::clock() - t);
  elapsed_sec["doAction"] += ((double)t/CLOCKS_PER_SEC) * 1000.0;
  called_counter["doAction"]++;
  return false;
}

bool Game::isPawnPromotion(pii curr_pos, pii new_pos) {
  if(!isAvailable(curr_pos, new_pos)) return false;

  int promotion_y = (isWhiteTurn() ? 0 : 7);

  return (isPawn(board[curr_pos.first][curr_pos.second]) && new_pos.second == promotion_y);
}

bool Game::drawConditions(const GameState &gs) const {
  // Repetition
  if(gs.repetition) return true;

  // Stalemate
  if(nextMoves.size() == 0) return true;
  // Insufficient mating material
  bool isInsufficient = true;
  int total_pieces = 0;
  for(int i=0;i<gs.pieces_counter.size();i++) {
    total_pieces += gs.pieces_counter[i];
  }

  if(total_pieces > 2) isInsufficient = false;
  else if(total_pieces == 2) {
    if(gs.pieces_counter[2] + gs.pieces_counter[8] != 2 && gs.pieces_counter[3] + gs.pieces_counter[9] != 2) isInsufficient = false;
  } else if(total_pieces == 1) {
    const std::vector<int> pos = {0, 4, 5, 6, 10, 11};
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

std::vector<std::pair<pii, pii>> Game::getAllMoves() {
  return nextMoves;
}

double Game::getScore() const {
  const GameState &gs = getState();

  if(gs.gameStatus == DRAW) return 0.0;
  else if(gs.gameStatus == CHECKMATE) {
    if(isWhiteTurn()) return -1000.0;
    else return 1000.0;
  }

  return gs.piecesScoring + gs.scoringHeuristic();
}

// Performance
void Game::performance() {
  std::cerr << "----------------------\n";
  std::cerr << "PERF ANALYSIS\n\n";
  for(const auto &data: elapsed_sec) {
    const std::string &f = data.first;
    int cnt = called_counter.at(f);
    double ms = (data.second / cnt);
    std::cerr << std::fixed << std::setprecision(3);

    std::cerr << "Function: " << f << "\n";
    std::cerr << "* Total time:     " << (data.second / (1000.0)) << "s\n";
    std::cerr << "* function calls: " << cnt << "\n";
    std::cerr << "* average time:   " << ms << "ms\n\n"; 
  }

  elapsed_sec.clear();
  called_counter.clear();
}

double Game::getCellScore(int x, int y) const {
  int info = getPositionInfo(x, y);
  assert(info != OUT);

  return evaluatePiece(info);
}
