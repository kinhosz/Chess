#include <Game.hpp>

Game::Game() {
  GameState gs;
  gs.gameStatus = "alive";
  gs.enPassant = {-1, -1};
  gs.castlingPreserved = 0;
  addState(gs);

  buildBoard();
  genNextMoves(gs);
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

std::vector<std::vector<std::string>> Game::getBoard(int move_id) const {
  if(move_id == -1) move_id = moves.size();

  std::vector<std::vector<std::string>> tmp = board;

  for(int i=(int)moves.size() - 1; i>=move_id; i--) {
    for(auto &c: moves[i]) {
      tmp[c.first.first][c.first.second] = c.second;
    }
  }

  return tmp;
}

std::string Game::getBoardHash() const {
  std::string hsh = "";
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      if(board[i][j] == "") hsh += "xx";
      else hsh += board[i][j];
    }
  }

  return hsh;
}

void Game::storeHashedBoard() {
  hashedBoardCounter[getBoardHash()]++;
}

std::vector<std::pair<pii, int>> Game::getSpecialCells(pii cell) const {
  std::vector<std::pair<pii, int>> cells;
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

pii Game::getKingPos(bool white) const {
  std::string king = (white ? "wk" : "bk");

  int king_x = -1, king_y = -1;
  for(int i=0;i<8 && king_x == -1;i++) {
    for(int j=0;j<8 && king_x == -1;j++) {
      if(board[i][j] == king) {
        king_x = i;
        king_y = j;
      }
    }
  }
  assert(king_x != -1);
  return {king_x, king_y};
}

bool Game::isDraw() const {
  return getState().gameStatus == "draw";
}

bool Game::isCheckMate() const {
  return getState().gameStatus == "checkmate";
}

void Game::buildBoard() {
  for(int i=0;i<8;i++) {
    std::vector<std::string> row;
    for(int j=0;j<8;j++) {
      row.push_back("");
    }
    board.push_back(row);
  }

  board[0][0] = "br";
  board[1][0] = "bn";
  board[2][0] = "bb";
  board[3][0] = "bq";
  board[4][0] = "bk";
  board[5][0] = "bb";
  board[6][0] = "bn";
  board[7][0] = "br";
  board[0][7] = "wr";
  board[1][7] = "wn";
  board[2][7] = "wb";
  board[3][7] = "wq";
  board[4][7] = "wk";
  board[5][7] = "wb";
  board[6][7] = "wn";
  board[7][7] = "wr";

  for(int i=0;i<8;i++) {
    board[i][1] = "bp";
    board[i][6] = "wp";
  }
  storeHashedBoard();
}

std::string Game::getPositionInfo(int x, int y) const {
  if(x < 0 || x > 7 || y < 0 || y > 7) return "out";
  return board[x][y];
}

bool Game::isOnCheck() const {
  pii king_pos = getKingPos(isWhiteTurn());
  int king_x = king_pos.first;
  int king_y = king_pos.second;

  // Checked by a Pawn
  if(isWhiteTurn()) {
    if(getPositionInfo(king_x-1, king_y-1) == "bp" || getPositionInfo(king_x+1, king_y-1) == "bp") return true;
  } else {
    if(getPositionInfo(king_x-1, king_y+1) == "wp" || getPositionInfo(king_x+1, king_y+1) == "wp") return true;
  }

  // Checked by a King
  int dl1[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc1[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  for(int i=0;i<8;i++) {
    std::string info = getPositionInfo(king_x + dl1[i], king_y + dc1[i]);
    if(isWhiteTurn() && info == "bk") return true;
    if(!isWhiteTurn() && info == "wk") return true;
  }

  // Checked by a Knight
  int dl2[] = {-2, -2, -1, 1, 2, 2, -1, 1};
  int dc2[] = {-1, 1, 2, 2, -1, 1, -2, -2};
  for(int i=0;i<8;i++) {
    std::string info = getPositionInfo(king_x + dl2[i], king_y + dc2[i]);
    if(isWhiteTurn() && info == "bn") return true;
    if(!isWhiteTurn() && info == "wn") return true;
  }

  // Checked by a Bishop / Queen
  int dl3[] = {-1, -1, 1, 1};
  int dc3[] = {-1, 1, 1, -1};
  for(int i=0;i<4;i++) {
    int t_king_x = king_x + dl3[i];
    int t_king_y = king_y + dc3[i];

    std::string info = getPositionInfo(t_king_x, t_king_y);
    while(info == "") {
      t_king_x += dl3[i];
      t_king_y += dc3[i];
      info = getPositionInfo(t_king_x, t_king_y);
    }
    if(isWhiteTurn() && (info == "bb" || info == "bq")) return true;
    if(!isWhiteTurn() && (info == "wb" || info == "wq")) return true;
  }

  // Checked by a Rook / Queen
  int dl4[] = {-1, 0, 1, 0};
  int dc4[] = {0, -1, 0, 1};
  for(int i=0;i<4;i++) {
    int t_king_x = king_x + dl4[i];
    int t_king_y = king_y + dc4[i];

    std::string info = getPositionInfo(t_king_x, t_king_y);
    while(info == "") {
      t_king_x += dl4[i];
      t_king_y += dc4[i];
      info = getPositionInfo(t_king_x, t_king_y);
    }
    if(isWhiteTurn() && (info == "br" || info == "bq")) return true;
    if(!isWhiteTurn() && (info == "wr" || info == "wq")) return true;
  }
  return false;
}

bool Game::isValidMove(pii curr_pos, pii new_pos) {
  std::string current_pos_before = getPositionInfo(curr_pos.first, curr_pos.second);
  std::string new_pos_before = getPositionInfo(new_pos.first, new_pos.second);

  if(new_pos_before == "out") return false;
  if(isWhiteTurn() && (new_pos_before != "" && new_pos_before[0] == 'w')) return false;
  if(!isWhiteTurn() && (new_pos_before != "" && new_pos_before[0] == 'b')) return false;

  // Move the piece
  board[curr_pos.first][curr_pos.second] = "";
  board[new_pos.first][new_pos.second] = current_pos_before;

  bool isValid = !isOnCheck();

  // Rollback board
  board[curr_pos.first][curr_pos.second] = current_pos_before;
  board[new_pos.first][new_pos.second] = new_pos_before;

  return isValid;
}

void Game::genNextMoves(const GameState gs) {
  nextMoves.clear();

  std::vector<std::pair<std::string, pii>> setup;
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      if(board[i][j] == "") continue;
      setup.push_back({board[i][j], {i, j}});
    }
  }

  for(int i=0;i<setup.size();i++) {
    if(isWhiteTurn() && setup[i].first[0] == 'b') continue;
    if(!isWhiteTurn() && setup[i].first[0] == 'w') continue;

    pii current_pos = setup[i].second;

    if(setup[i].first[1] == 'n') {
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
    if(setup[i].first[1] == 'k') {
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
      if(isWhiteTurn() && gs.isCastlingPreserved(0) && getPositionInfo(0, 7) == "wr"
        && getPositionInfo(1, 7) == "" && getPositionInfo(2, 7) == "" && getPositionInfo(3, 7) == "" && !isOnCheck()) {

        if(isValidMove({4, 7}, {3, 7}) && isValidMove({4, 7}, {2, 7})) {
          nextMoves.push_back({{4, 7}, {2, 7}});
        } 
      }
      // Black Castling: left side
      if(!isWhiteTurn() && gs.isCastlingPreserved(2) && getPositionInfo(0, 0) == "br"
        && getPositionInfo(1, 0) == "" && getPositionInfo(2, 0) == "" && getPositionInfo(3, 0) == "" &&  !isOnCheck()) {

        if(isValidMove({4, 0}, {3, 0}) && isValidMove({4, 0}, {2, 0})) {
          nextMoves.push_back({{4, 0}, {2, 0}});
        } 
      }
      if(isWhiteTurn() && gs.isCastlingPreserved(1) && getPositionInfo(7, 7) == "wr"
        && getPositionInfo(5, 7) == "" && getPositionInfo(6, 7) == "" && !isOnCheck()) {

        if(isValidMove({4, 7}, {5, 7}) && isValidMove({4, 7}, {6, 7})) {
          nextMoves.push_back({{4, 7}, {6, 7}});
        }
      }
      // Black Castling: right side
      if(!isWhiteTurn() && gs.isCastlingPreserved(3) && getPositionInfo(7, 0) == "br"
        && getPositionInfo(5, 0) == "" && getPositionInfo(6, 0) == "" && !isOnCheck()) {

        if(isValidMove({4, 0}, {5, 0}) && isValidMove({4, 0}, {6, 0})) {
          nextMoves.push_back({{4, 0}, {6, 0}});
        }
      }
    }
    if(setup[i].first[1] == 'r' || setup[i].first[1] == 'q') {
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

          if(getPositionInfo(new_pos.first, new_pos.second) != "") break;
        }
      }
    }
    if(setup[i].first[1] == 'b' || setup[i].first[1] == 'q') {
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

          if(getPositionInfo(new_pos.first, new_pos.second) != "") break;
        }
      }
    }
    if(setup[i].first[1] == 'p') {
      int front_direction = (isWhiteTurn() ? -1 : 1);
      int initial_row = (7 + front_direction) % 7;

      // Left taking
      if(isValidMove(current_pos, {current_pos.first - 1, current_pos.second + front_direction})) {
        std::string info = getPositionInfo(current_pos.first - 1, current_pos.second + front_direction);
        if(info != "" && info[0] != setup[i].first[0]) {
          nextMoves.push_back({current_pos, {current_pos.first - 1, current_pos.second + front_direction}});
        }
      }
      // Right taking
      if(isValidMove(current_pos, {current_pos.first + 1, current_pos.second + front_direction})) {
        std::string info = getPositionInfo(current_pos.first + 1, current_pos.second + front_direction);
        if(info != "" && info[0] != setup[i].first[0]) {
          nextMoves.push_back({current_pos, {current_pos.first + 1, current_pos.second + front_direction}});
        }
      }
      // En passant
      if(gs.enPassant == std::make_pair(current_pos.first - 1, current_pos.second)
        || gs.enPassant == std::make_pair(current_pos.first + 1, current_pos.second)) {

        std::string attacker = board[current_pos.first][current_pos.second];
        std::string deffensor = board[gs.enPassant.first][gs.enPassant.second];

        board[current_pos.first][current_pos.second] = "";
        board[gs.enPassant.first][gs.enPassant.second] = "";
        board[gs.enPassant.first][gs.enPassant.second + front_direction] = attacker;

        if(!isOnCheck()) {
          nextMoves.push_back({current_pos, {gs.enPassant.first, gs.enPassant.second + front_direction}});
        }

        board[current_pos.first][current_pos.second] = attacker;
        board[gs.enPassant.first][gs.enPassant.second] = deffensor;
        board[gs.enPassant.first][gs.enPassant.second + front_direction] = "";
      }
      // Two moves
      if(current_pos.second == initial_row) {
        if(board[current_pos.first][current_pos.second + front_direction] == ""
          && board[current_pos.first][current_pos.second + 2 * front_direction] == "") {

          if(isValidMove(current_pos, {current_pos.first, current_pos.second + 2 * front_direction})) {
            nextMoves.push_back({current_pos, {current_pos.first, current_pos.second + 2 * front_direction}});
          }
        }
      }
      // Single move
      if(getPositionInfo(current_pos.first, current_pos.second + front_direction) == "") {
        if(isValidMove(current_pos, {current_pos.first, current_pos.second + front_direction})) {
          nextMoves.push_back({current_pos, {current_pos.first, current_pos.second + front_direction}});
        }
      }
    }
  }
}

double Game::evaluatePiece(std::string piece) const {
  if(piece == "") return 0.0;
  int mult = (piece[0] == 'w' ? 1.0 : -1.0);

  double value = 0.0;

  if(piece[1] == 'r') value = 5.0;
  else if(piece[1] == 'n') value = 3.0;
  else if(piece[1] == 'b') value = 3.0;
  else if(piece[1] == 'q') value = 9.0;
  else if(piece[1] == 'p') value = 1.0;

  return mult * value;
}

double Game::executeMove(std::vector<std::pair<pii, std::string>> &move) {
  std::vector<std::pair<pii, std::string>> rollback;
  double score = 0.0;

  for(auto &m: move) {
    std::string curr_piece = board[m.first.first][m.first.second];
    rollback.push_back({m.first, curr_piece});
    board[m.first.first][m.first.second] = m.second;

    score -= evaluatePiece(curr_piece);
    score += evaluatePiece(m.second);
  }

  moves.push_back(rollback);

  return score;
}

void Game::undoAction() {
  gameState.pop_back();

  hashedBoardCounter[getBoardHash()]--;

  auto &undo_move = moves.back();
  for(auto &m: undo_move) {
    board[m.first.first][m.first.second] = m.second;
  }
  moves.pop_back();

  genNextMoves(gameState.back());
}

double Game::doAction(pii current_pos, pii new_pos, int choose) {
  assert(isAvailable(current_pos, new_pos));

  const GameState curr_gs = getState();
  GameState new_gs = curr_gs;
  new_gs.enPassant = {-1, -1};

  std::string piece = board[current_pos.first][current_pos.second];
  std::vector<std::pair<pii, std::string>> current_move;

  if(piece[1] == 'p' && board[new_pos.first][new_pos.second] == "" && current_pos.first != new_pos.first) {
    // Action: En passant
    current_move.push_back({{current_pos.first, current_pos.second}, ""});
    current_move.push_back({{new_pos.first, new_pos.second}, piece});
    current_move.push_back({{curr_gs.enPassant.first, curr_gs.enPassant.second}, ""});

  } else if(piece[1] == 'k' && int(std::abs(current_pos.first - new_pos.first)) == 2) {
    // Action: Castling
    int row = current_pos.second;
    if(new_pos.first == 2) {
      std::string rook = board[0][row];

      current_move.push_back({{0, row}, ""});
      current_move.push_back({{2, row}, piece});
      current_move.push_back({{3, row}, rook});
      current_move.push_back({{4, row}, ""});

    } else {
      std::string rook = board[7][row];

      current_move.push_back({{7, row}, ""});
      current_move.push_back({{6, row}, piece});
      current_move.push_back({{5, row}, rook});
      current_move.push_back({{4, row}, ""});

    }

    if(isWhiteTurn()) {
      new_gs.touch(0);
      new_gs.touch(1);
    } else {
      new_gs.touch(2);
      new_gs.touch(3);
    }
  } else if(piece[1] == 'p' && int(std::abs(current_pos.second - new_pos.second)) == 2) {
    // Action: Two moves
    new_gs.enPassant = new_pos;

    current_move.push_back({{current_pos.first, current_pos.second}, ""});
    current_move.push_back({{new_pos.first, new_pos.second}, piece});

  } else if(piece[1] == 'p' && (new_pos.second == 0 || new_pos.second == 7)) {
    // Action: Promotion
    assert(choose != -1);
    std::string promotedPiece = (isWhiteTurn() ? "w" : "b");
    if(choose == 0) promotedPiece += "q";
    else if(choose == 1) promotedPiece += "r";
    else if(choose == 2) promotedPiece += "n";
    else if(choose == 3) promotedPiece += "b";

    current_move.push_back({{current_pos.first, current_pos.second}, ""});
    current_move.push_back({{new_pos.first, new_pos.second}, promotedPiece});

  } else {
    // Any other move
    current_move.push_back({{current_pos.first, current_pos.second}, ""});
    current_move.push_back({{new_pos.first, new_pos.second}, piece});

  }
  double score = executeMove(current_move);
  storeHashedBoard();

  if(piece == "wk") new_gs.touch(0), new_gs.touch(1);
  if(piece == "bk") new_gs.touch(2), new_gs.touch(3);
  if(piece == "wr" && current_pos.first == 0) new_gs.touch(0);
  if(piece == "wr" && current_pos.first == 7) new_gs.touch(1);
  if(piece == "br" && current_pos.first == 0) new_gs.touch(2);
  if(piece == "br" && current_pos.first == 7) new_gs.touch(3);

  genNextMoves(new_gs);

  if(drawConditions()) new_gs.gameStatus = "draw";
  if(new_gs.gameStatus == "draw" && isOnCheck()) new_gs.gameStatus = "checkmate";

  addState(new_gs);

  if(new_gs.gameStatus == "draw") return 0.0;
  else if(new_gs.gameStatus == "checkmate") {
    if(isWhiteTurn()) return 1.000;
    else return -1.000;
  }

  return score;
}

bool Game::hasMoveFor(pii pos) const {
  for(int i=0;i<nextMoves.size();i++) {
    if(nextMoves[i].first == pos) return true;
  }
  return false;
}

bool Game::isAvailable(pii curr_pos, pii new_pos) const {
  for(int i=0;i<nextMoves.size();i++) {
    if(nextMoves[i].first == curr_pos && nextMoves[i].second == new_pos) return true;
  }
  return false;
}

bool Game::isPawnPromotion(pii curr_pos, pii new_pos) const {
  if(!isAvailable(curr_pos, new_pos)) return false;

  int promotion_y = (isWhiteTurn() ? 0 : 7);

  return (board[curr_pos.first][curr_pos.second][1] == 'p' && new_pos.second == promotion_y);
}

bool Game::drawConditions() const {
  // Stalemate
  if(nextMoves.size() == 0) return true;
  // Insufficient mating material
  int material = 0;
  /*********************
  0: nothing
  1: knight
  2: white bishop
  3: black bishop
  **********************/
  bool isInsufficient = true;
  for(int i=0;i<8 && isInsufficient;i++) {
    for(int j=0;j<8 && isInsufficient;j++) {
      if(board[i][j] == "" || board[i][j][1] == 'k') continue;
      else if(board[i][j][1] == 'p' || board[i][j][1] == 'q' || board[i][j][1] == 'r') isInsufficient = false;
      else if(board[i][j][1] == 'n') {
        if(material == 0) material = 1;
        else isInsufficient = false;
      } else {
        int c = ((j + (i % 2)) % 2 == 0 ? 2 : 3);
        if(material == 0) material = c;
        else if(material != c) isInsufficient = false;
      }
    }
  }
  if(isInsufficient) return true;

  // Repetition
  for(auto &p: hashedBoardCounter) {
    if(p.second >= 3) return true;
  }

  // All checks have been passed
  return false;
}

int Game::getTotalMoves() const {
  return moves.size();
}

std::vector<std::pair<pii, pii>> Game::getAllMoves() const {
  return nextMoves;
}
