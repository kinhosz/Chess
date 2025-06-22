#include <Game.hpp>

Game::Game() {
  state = 0;
  whiteTurn = true;
  enPassant = {-1, -1};
  isLeftWhiteRookMoved = false;
  isRightWhiteRookMoved = false;
  isWhiteKingMoved = false;
  isLeftBlackRookMoved = false;
  isRightBlackRookMoved = false;
  isBlackKingMoved = false;

  buildBoard();
  genNextMoves();
}

std::vector<std::vector<std::string>> Game::getBoard() const {
  return board;
}

std::vector<std::pair<pii, int>> Game::getSpecialCells(pii cell) const {
  std::vector<std::pair<pii, int>> cells;
  if(isDraw()) {
    cells.push_back({getKingPos(true), -1});
    cells.push_back({getKingPos(false), -1});
  } else if(isCheckMate()) {
    cells.push_back({getKingPos(whiteTurn), 1});
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
  return state == 1;
}

bool Game::isCheckMate() const {
  return state == 2;
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
}

std::string Game::getPositionInfo(int x, int y) const {
  if(x < 0 || x > 7 || y < 0 || y > 7) return "out";
  return board[x][y];
}

bool Game::isOnCheck() const {
  pii king_pos = getKingPos(whiteTurn);
  int king_x = king_pos.first;
  int king_y = king_pos.second;

  // Checked by a Pawn
  if(whiteTurn) {
    if(getPositionInfo(king_x-1, king_y-1) == "bp" || getPositionInfo(king_x+1, king_y-1) == "bp") return true;
  } else {
    if(getPositionInfo(king_x-1, king_y+1) == "wp" || getPositionInfo(king_x+1, king_y+1) == "wp") return true;
  }

  // Checked by a King
  int dl1[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dc1[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  for(int i=0;i<8;i++) {
    std::string info = getPositionInfo(king_x + dl1[i], king_y + dc1[i]);
    if(whiteTurn && info == "bk") return true;
    if(!whiteTurn && info == "wk") return true;
  }

  // Checked by a Knight
  int dl2[] = {-2, -2, -1, 1, 2, 2, -1, 1};
  int dc2[] = {-1, 1, 2, 2, -1, 1, -2, -2};
  for(int i=0;i<8;i++) {
    std::string info = getPositionInfo(king_x + dl2[i], king_y + dc2[i]);
    if(whiteTurn && info == "bn") return true;
    if(!whiteTurn && info == "wn") return true;
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
    if(whiteTurn && (info == "bb" || info == "bq")) return true;
    if(!whiteTurn && (info == "wb" || info == "wq")) return true;
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
    if(whiteTurn && (info == "br" || info == "bq")) return true;
    if(!whiteTurn && (info == "wr" || info == "wq")) return true;
  }
  return false;
}

bool Game::isValidMove(pii curr_pos, pii new_pos) {
  std::string current_pos_before = getPositionInfo(curr_pos.first, curr_pos.second);
  std::string new_pos_before = getPositionInfo(new_pos.first, new_pos.second);

  if(new_pos_before == "out") return false;
  if(whiteTurn && (new_pos_before != "" && new_pos_before[0] == 'w')) return false;
  if(!whiteTurn && (new_pos_before != "" && new_pos_before[0] == 'b')) return false;

  // Move the piece
  board[curr_pos.first][curr_pos.second] = "";
  board[new_pos.first][new_pos.second] = current_pos_before;

  bool isValid = !isOnCheck();

  // Rollback board
  board[curr_pos.first][curr_pos.second] = current_pos_before;
  board[new_pos.first][new_pos.second] = new_pos_before;

  return isValid;
}

void Game::genNextMoves() {
  nextMoves.clear();

  std::vector<std::pair<std::string, pii>> setup;
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      if(board[i][j] == "") continue;
      setup.push_back({board[i][j], {i, j}});
    }
  }

  for(int i=0;i<setup.size();i++) {
    if(whiteTurn && setup[i].first[0] == 'b') continue;
    if(!whiteTurn && setup[i].first[0] == 'w') continue;

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
      if(whiteTurn && !isWhiteKingMoved && !isLeftWhiteRookMoved
        && getPositionInfo(1, 7) == "" && getPositionInfo(2, 7) == "" && getPositionInfo(3, 7) == "" && !isOnCheck()) {

        if(isValidMove({4, 7}, {3, 7}) && isValidMove({4, 7}, {2, 7})) {
          nextMoves.push_back({{4, 7}, {2, 7}});
        } 
      }
      // Black Castling: left side
      if(!whiteTurn && !isBlackKingMoved && !isLeftBlackRookMoved
        && getPositionInfo(1, 0) == "" && getPositionInfo(2, 0) == "" && getPositionInfo(3, 0) == "" &&  !isOnCheck()) {

        if(isValidMove({4, 0}, {3, 0}) && isValidMove({4, 0}, {2, 0})) {
          nextMoves.push_back({{4, 0}, {2, 0}});
        } 
      }
      if(whiteTurn && !isWhiteKingMoved && !isRightWhiteRookMoved
        && getPositionInfo(5, 7) == "" && getPositionInfo(6, 7) == "" && !isOnCheck()) {

        if(isValidMove({4, 7}, {5, 7}) && isValidMove({4, 7}, {6, 7})) {
          nextMoves.push_back({{4, 7}, {6, 7}});
        }
      }
      // Black Castling: right side
      if(!whiteTurn && !isBlackKingMoved && !isRightBlackRookMoved
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
      int front_direction = (whiteTurn ? -1 : 1);
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
      if(enPassant == std::make_pair(current_pos.first - 1, current_pos.second)
        || enPassant == std::make_pair(current_pos.first + 1, current_pos.second)) {

        std::string attacker = board[current_pos.first][current_pos.second];
        std::string deffensor = board[enPassant.first][enPassant.second];

        board[current_pos.first][current_pos.second] = "";
        board[enPassant.first][enPassant.second] = "";
        board[enPassant.first][enPassant.second + front_direction] = attacker;

        if(!isOnCheck()) {
          nextMoves.push_back({current_pos, {enPassant.first, enPassant.second + front_direction}});
        }

        board[current_pos.first][current_pos.second] = attacker;
        board[enPassant.first][enPassant.second] = deffensor;
        board[enPassant.first][enPassant.second + front_direction] = "";
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

void Game::resetEnPassant() {
  enPassant = {-1, -1};
}

void Game::doAction(pii current_pos, pii new_pos, int choose) {
  std::string piece = board[current_pos.first][current_pos.second];

  if(piece[1] == 'p' && board[new_pos.first][new_pos.second] == "" && current_pos.first != new_pos.first) {
    // Action: En passant
    board[current_pos.first][current_pos.second] = "";
    board[new_pos.first][new_pos.second] = piece;
    board[enPassant.first][enPassant.second] = "";
    resetEnPassant();
  } else if(piece[1] == 'k' && int(std::abs(current_pos.first - new_pos.first)) == 2) {
    // Action: Castling
    int row = current_pos.second;
    if(new_pos.first == 2) {
      std::string rook = board[0][row];

      board[0][row] = "";
      board[2][row] = piece;
      board[3][row] = rook;
      board[4][row] = "";

      if(whiteTurn) isLeftWhiteRookMoved = true;
      else isLeftBlackRookMoved = true;
    } else {
      std::string rook = board[7][row];

      board[7][row] = "";
      board[6][row] = piece;
      board[5][row] = rook;
      board[4][row] = "";

      if(whiteTurn) isRightWhiteRookMoved = true;
      else isRightBlackRookMoved = true;
    }
    if(whiteTurn) isWhiteKingMoved = true;
    else isBlackKingMoved = true;
    resetEnPassant();
  } else if(piece[1] == 'p' && int(std::abs(current_pos.second - new_pos.second)) == 2) {
    // Action: Two moves
    enPassant = new_pos;
    board[current_pos.first][current_pos.second] = "";
    board[new_pos.first][new_pos.second] = piece;
  } else if(piece[1] == 'p' && (new_pos.second == 0 || new_pos.second == 7)) {
    // Action: Promotion
    assert(choose != -1);
    std::string promotedPiece = (whiteTurn ? "w" : "b");
    if(choose == 0) promotedPiece += "q";
    else if(choose == 1) promotedPiece += "r";
    else if(choose == 2) promotedPiece += "n";
    else if(choose == 3) promotedPiece += "b";

    board[current_pos.first][current_pos.second] = "";
    board[new_pos.first][new_pos.second] = promotedPiece;

    resetEnPassant();
  } else {
    // Any other move
    board[current_pos.first][current_pos.second] = "";
    board[new_pos.first][new_pos.second] = piece;
    resetEnPassant();
  }

  if(piece == "wk") isWhiteKingMoved = true;
  if(piece == "bk") isBlackKingMoved = true;
  if(piece == "wr" && current_pos.first == 0) isLeftWhiteRookMoved = true;
  if(piece == "wr" && current_pos.first == 7) isRightWhiteRookMoved = true;
  if(piece == "br" && current_pos.first == 0) isLeftBlackRookMoved = true;
  if(piece == "br" && current_pos.first == 7) isRightBlackRookMoved = true;

  whiteTurn = !whiteTurn;
  genNextMoves();

  if(drawConditions()) state = 1;
  if(state == 1 && isOnCheck()) state = 2;
}

bool Game::isWhiteTurn() const {
  return whiteTurn;
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

  int promotion_y = (whiteTurn ? 0 : 7);

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

  // All checks have been passed
  return false;
}
