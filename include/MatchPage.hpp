#ifndef MATCHPAGE_HPP
#define MATCHPAGE_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Font.hpp>

#include <Button.hpp>
#include <Game.hpp>
#include <Engine.hpp>
#include <Profiler.hpp>

class MatchPage {
private:
  int WIDTH, HEIGHT;
  float PADDING = 50.f;
  float SQUARE_SIZE = 100.f;
  bool showPromotionSquare;
  vi2 move;
  std::vector<Button> buttons;

  Game game;
  int move_counter;

  int MATCH_MODE = 3;
  Engine engine;
  int DEEP_SIZE = 6;

  bool force_refresh = false;

  void createButtons() {
    // Board cells
    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        int x0 = PADDING + i * SQUARE_SIZE;
        int y0 = PADDING + j * SQUARE_SIZE;
        int xf = x0 + SQUARE_SIZE;
        int yf = y0 + SQUARE_SIZE;
        buttons.push_back(Button(x0, xf, y0, yf));
        buttons[buttons.size()-1].setGroup("board");
      }
    }
    // Promotion cells
    for(int i=0;i<4;i++) {
      float offset_x = PADDING + 8.0 * SQUARE_SIZE + PADDING;
      float offset_y = PADDING;

      float x0 = offset_x;
      float xf = x0 + SQUARE_SIZE;
      float y0 = PADDING + i * SQUARE_SIZE;
      float yf = y0 + SQUARE_SIZE;
      buttons.push_back(Button(x0, xf, y0, yf));
      buttons[buttons.size()-1].setGroup("promotion");
    }
    // Left & Right arrows
    float x0 = 2.0 * PADDING + 8.0 * SQUARE_SIZE;
    float y0 = PADDING + 7.0 * SQUARE_SIZE;
    buttons.push_back(Button(x0, x0 + SQUARE_SIZE, y0, y0 + SQUARE_SIZE));
    buttons[buttons.size()-1].setGroup("action");
    buttons[buttons.size()-1].setName("previous-move");

    x0 += SQUARE_SIZE;
    buttons.push_back(Button(x0, x0 + SQUARE_SIZE, y0, y0 + SQUARE_SIZE));
    buttons[buttons.size()-1].setGroup("action");
    buttons[buttons.size()-1].setName("next-move");

    // Debugger
    x0 += SQUARE_SIZE;
    buttons.push_back(Button(x0, x0 + SQUARE_SIZE / 4, y0, y0 + SQUARE_SIZE / 4));
    buttons[buttons.size()-1].setGroup("action");
    buttons[buttons.size()-1].setName("debugger");
  }

  sf::RectangleShape createSquare(float x, float y, sf::Color c) {
    sf::RectangleShape rect({SQUARE_SIZE, SQUARE_SIZE});
    rect.setFillColor(c);
    rect.setPosition({x, y});
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(0, 0, 0));

    return rect;
  }

  void drawText(sf::RenderWindow &window) {
    std::string msg = "Bot is thinking...";

    sf::Font font("assets/fonts/bitcount_prop_single.ttf");
    sf::Text text(font);
    text.setString(msg);
    text.setCharacterSize(24);
    text.setFillColor(sf::Color(225, 225, 230));
    text.setStyle(sf::Text::Bold);
    text.setPosition({50 , 10});
    window.draw(text);
  }

  void drawBoard(sf::RenderWindow &window) {
    sf::RectangleShape rect({WIDTH, HEIGHT});
    rect.setFillColor(sf::Color(24, 26, 33));
    rect.setPosition({0, 0});
    window.draw(rect);

    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        sf::Color c(214, 200, 170);
        if((i + j)%2 == 1) c = sf::Color(0, 150, 0);
        window.draw(createSquare(buttons[i * 8 + j].x0, buttons[i * 8 + j].y0, c));
      }
    }

    i2 cell = {-1, -1};
    if(move.size() > 0) cell = move[0];

    vi3 specialCells = game.getSpecialCells(cell);
    for(int i=0;i<specialCells.size();i++) {
      int x = specialCells[i].first.first;
      int y = specialCells[i].first.second;
      int info = specialCells[i].second;

      sf::Color c(218, 154, 44); // Piece moved
      if(info == 1) {
        c = sf::Color(200, 0, 0); // CheckMate
      } else if(info == -1) {
        c = sf::Color(100, 100, 100); // Draw
      } else if(info == 2) {
        c = sf::Color(180, 130, 20); // Assigned Piece
      } else if(info == 3) {
        c = sf::Color(0, 153, 255); // Last move
      }

      window.draw(createSquare(buttons[x * 8 + y].x0, buttons[x * 8 + y].y0, c));
    }
  }

  void drawCoordinates(sf::RenderWindow &window) {
    sf::Font font("assets/fonts/bitcount_prop_single.ttf");
    const std::string files = "abcdefgh";

    for(int i=0;i<8;i++) {
      sf::Text text(font);
      text.setString(std::string(1, files[i]));
      text.setCharacterSize(16);
      text.setFillColor(sf::Color(225, 225, 230));
      text.setPosition({PADDING + i * SQUARE_SIZE + SQUARE_SIZE / 2 - 5, PADDING + 8 * SQUARE_SIZE + 6});
      window.draw(text);
    }

    for(int j=0;j<8;j++) {
      sf::Text text(font);
      text.setString(std::to_string(8 - j)); // row j=0 is rank 8 (black's back rank), row j=7 is rank 1
      text.setCharacterSize(16);
      text.setFillColor(sf::Color(225, 225, 230));
      text.setPosition({PADDING - 22, PADDING + j * SQUARE_SIZE + SQUARE_SIZE / 2 - 10});
      window.draw(text);
    }
  }

  // Captured pieces = how many of each type are missing from the board
  // compared to the starting setup; no new Game API needed, getBoard() is
  // already public.
  void drawCapturedPieces(sf::RenderWindow &window) {
    const std::vector<std::vector<int>> &setup = game.getBoard(move_counter);
    std::map<int, int> onBoard;
    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        if(setup[i][j] != EMPTY) onBoard[setup[i][j]]++;
      }
    }

    std::vector<std::pair<int,int>> startingCounts = {
      {WQ,1},{WR,2},{WB,2},{WN,2},{WP,8}, {BQ,1},{BR,2},{BB,2},{BN,2},{BP,8}
    };

    float iconSize = 24.f;
    float x0 = PADDING + 8 * SQUARE_SIZE + PADDING;
    float yWhiteCaptured = PADDING + 4 * SQUARE_SIZE + 20; // black pieces white has captured
    float yBlackCaptured = yWhiteCaptured + iconSize + 10;  // white pieces black has captured

    float xWhite = x0, xBlack = x0;
    for(auto &sc: startingCounts) {
      int captured = sc.second - onBoard[sc.first];
      float &x = isWhite(sc.first) ? xBlack : xWhite; // captured white piece -> shown on black's tally
      float y = isWhite(sc.first) ? yBlackCaptured : yWhiteCaptured;
      for(int k=0;k<captured;k++) {
        drawPiece(window, getPieceName(sc.first), x, y, iconSize * 0.7f / SQUARE_SIZE);
        x += iconSize;
      }
    }
  }

  void drawActionButtons(sf::RenderWindow &window) {
    sf::Texture texture;
    int offset_id = 8*8 + 4;
    sf::Color c(180, 100, 50);
  
    // Left arrow
    window.draw(createSquare(buttons[offset_id].x0, buttons[offset_id].y0, c));
    std::string path = "assets/icons/left-arrow.png";
    if(!texture.loadFromFile(path.c_str())) {
      std::cerr << "Failed to open: " << path << "\n";
    }
    sf::Sprite sprite(texture);
    sprite.setScale({0.15f, 0.15f});
    sprite.setPosition({buttons[offset_id].x0 + 10.0, buttons[offset_id].y0 + 10.0});
    window.draw(sprite);

    // Right arrow
    window.draw(createSquare(buttons[offset_id + 1].x0, buttons[offset_id + 1].y0, c));
    path = "assets/icons/right-arrow.png";
    if(!texture.loadFromFile(path.c_str())) {
      std::cerr << "Failed to open: " << path << "\n";
    }
    sprite = sf::Sprite(texture);
    sprite.setScale({0.15f, 0.15f});
    sprite.setPosition({buttons[offset_id + 1].x0 + 10.0, buttons[offset_id + 1].y0 + 10.0});
    window.draw(sprite);
  }

  void drawPiece(sf::RenderWindow &window, std::string piece, float x, float y, float scale=0.7f) {
    sf::Texture texture;
    std::string path = "assets/pieces/" + piece + ".png";
    if(!texture.loadFromFile(path.c_str())) {
      std::cerr << "Failed to open: " << path << "\n";
    }
    sf::Sprite sprite(texture);
    sprite.setScale({scale, scale});

    sprite.setPosition({x, y});
    window.draw(sprite);
  }

  void drawPieces(sf::RenderWindow &window) {
    const std::vector<std::vector<int>> &setup = game.getBoard(move_counter);
    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        if(setup[i][j] == EMPTY) continue;
        drawPiece(window, getPieceName(setup[i][j]), PADDING + i * SQUARE_SIZE, PADDING + j * SQUARE_SIZE);
      }
    }
  }

  void drawPromotionOption(sf::RenderWindow &window) {
    std::string piece_color = (game.isWhiteTurn() ? "w" : "b");
    
    float offset_x = PADDING + 8.0 * SQUARE_SIZE + PADDING;
    float offset_y = PADDING; 
    for(int i=0;i<4;i++) {
      sf::Color c(50, 50, 180);
      std::string p;
      if(i == 0) p = "q";
      else if(i == 1) p = "r";
      else if(i == 2) p = "n";
      else if(i == 3) p = "b";

      std::string piece = piece_color + p;
      window.draw(createSquare(offset_x, offset_y + i * SQUARE_SIZE, c));
      drawPiece(window, piece, offset_x, offset_y + i * SQUARE_SIZE);
    }
  }

  void doGameMove(i2 curr_pos, i2 new_pos, int choose=-1) {
    bool wasWhiteTurn = game.isWhiteTurn();

    game.doAction(curr_pos, new_pos, choose);
    engine.moveDone({{curr_pos, new_pos}, choose});
    move_counter = game.getTotalMoves();
    force_refresh = true;

    std::cerr << "[GAME][MOVE] " << (move_counter - 1) << " (" << (wasWhiteTurn ? "white" : "black") << "): "
               << squareName(curr_pos) << squareName(new_pos);
    if(choose != -1) std::cerr << "=" << "qrnb"[choose];
    std::cerr << "\n";
  }

  void handlePromotion(int button_id) {
    assert(move.size() == 2);
    doGameMove(move[0], move[1], button_id - 64);
    showPromotionSquare = false;
    move.clear();
  }

  void handleAction(int button_id) {
    if(buttons[button_id].getName() == "previous-move") {
      move.clear();
      move_counter = std::max(0, move_counter - 1);
    } else if(buttons[button_id].getName() == "next-move") {
      move.clear();
      move_counter = std::min(game.getTotalMoves(), move_counter + 1);
    } else if(buttons[button_id].getName() == "debugger") {
      game.debugger();
    }
  }

  void handleBoardClick(int button_id) {
    int i = button_id / 8;
    int j = button_id % 8;
    if(move.size() == 0) {
      if(game.hasMoveFor({i, j})) move.push_back({i, j}); // Piece selection: Preventing for move
    } else {
      if(game.isAvailable(move[0], {i, j})) {
        move.push_back({i, j});
        if(game.isPawnPromotion(move[0], move[1])) {
          showPromotionSquare = true; // Waiting for promoted selection
        } else {
          doGameMove(move[0], move[1]); // Executing move
          move.clear();
        }
      } else {
        move.clear(); // Canceling move action
      }
    }
  }

  void logMoveExplanation(const MoveExplanation &explanation, int topN) {
    std::cerr << "[GAME][EXPLAIN] top candidates:\n";
    for(int i=0;i<topN && i<(int)explanation.candidates.size();i++) {
      const auto &c = explanation.candidates[i];
      std::cerr << "  " << (i+1) << ". " << squareName(c.move.first.first) << squareName(c.move.first.second)
                 << " promo=" << c.move.second << "  score=" << c.score << "\n";
    }
    std::cerr << "[GAME][EXPLAIN] expected continuation:";
    for(auto &m: explanation.principalVariation) {
      std::cerr << " " << squareName(m.first.first) << squareName(m.first.second);
    }
    std::cerr << "\n";
  }

  void botAction() {
    if(isPlayerTurn()) return;
    if(game.isCheckMate() || game.isDraw()) return;

    MoveExplanation explanation;
    i5 move = engine.getNextMove(DEEP_SIZE, nullptr, &explanation);
    doGameMove(move.first.first, move.first.second, move.second);

    Profiler::getInstance().logAll();
    Profiler::getInstance().logMemory("after bot move");
    std::cerr << "[GAME][SCORE] " << game.getScore() << "\n";
    logMoveExplanation(explanation, 5);
  }

public:
  MatchPage() {}

  MatchPage(int width, int height, int match_mode=3, int deep_size=6) {
    WIDTH = width;
    HEIGHT = height;
    MATCH_MODE = match_mode;
    DEEP_SIZE = deep_size;
    showPromotionSquare = false;
    move_counter = 0;
    createButtons();

    // Logged once so a pasted [GAME][MOVE]/[GAME][EXPLAIN] transcript is
    // self-contained -- reproducing a game needs the exact depth and seed,
    // not just the move list.
    std::cerr << "[GAME][CONFIG] depth=" << DEEP_SIZE << " match_mode=" << MATCH_MODE
               << " seed=" << CHESS_RNG_SEED_USED << "\n";
  }

  void refresh(sf::RenderWindow &window) {
    /* Refresh the display */
    drawBoard(window);
    drawCoordinates(window);
    drawPieces(window);
    drawCapturedPieces(window);
    drawActionButtons(window);
    if(showPromotionSquare) drawPromotionOption(window);

    if(force_refresh) force_refresh = false;
    else botAction();

    if(!isPlayerTurn()) drawText(window);
  }

  bool isPlayerTurn() const {
    return ((MATCH_MODE&(1<<game.isWhiteTurn())) == 0);
  }

  bool canHandlePromotion(Button &b, double mx, double my) const {
    if(!showPromotionSquare) return false;

    if(!isPlayerTurn()) return false;

    if(b.getGroup() != "promotion") return false;

    if(!b.isClicked(mx, my)) return false;

    return true;
  }

  bool canHandleAction(Button &b, double mx, double my) const {
    if(showPromotionSquare) return false;

    if(b.getGroup() != "action") return false;

    if(!b.isClicked(mx, my)) return false;

    return true;
  }

  bool canHandleBoard(Button &b, double mx, double my) const {
    if(showPromotionSquare) return false;

    if(b.getGroup() != "board") return false;

    if(!isPlayerTurn()) return false;

    if(!b.isClicked(mx, my)) return false;
  
    if(game.getTotalMoves() != move_counter) return false;

    return true;
  }

  void handleClick(const sf::Event::MouseButtonPressed *event) {
    int mouse_x = event->position.x;
    int mouse_y = event->position.y;

    int button_id = -1;

    for(int i=0;i<buttons.size();i++) {
      /*
        The page has 3 main states:
          1) Promotion time
          2) History: Previous moves
          3) basic game move
      */

      // Promotion time: Only click on promotion options are availables
      if(canHandlePromotion(buttons[i], mouse_x, mouse_y)) {
        handlePromotion(i);
        break;
      }

      // Actions buttons are available at any time now
      if(canHandleAction(buttons[i], mouse_x, mouse_y)) {
        handleAction(i);
        break;
      }

      // Basic board click
      if(canHandleBoard(buttons[i], mouse_x, mouse_y)) {
        handleBoardClick(i);
        break;
      }
    }
  }
};

#endif
