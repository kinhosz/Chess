#ifndef MATCHPAGE_HPP
#define MATCHPAGE_HPP

#include <SFML/Graphics.hpp>

#include <Game.hpp>

class Button {
  std::string group = "";
  std::string name = "";
public:
  float x0, xf, y0, yf;

  Button(float x0, float xf, float y0, float yf) {
    this->x0 = x0;
    this->xf = xf;
    this->y0 = y0;
    this->yf = yf;
  }

  void setGroup(std::string group) {
    this->group = group;
  }

  std::string getGroup() const {
    return group;
  }

  void setName(std::string name) {
    this->name = name;
  }

  std::string getName() const {
    return name;
  }

  bool isClicked(int x, int y) {
    return x >= x0 && x <= xf && y >= y0 && y <= yf;
  }
};

class MatchPage {
private:
  int WIDTH, HEIGHT;
  float PADDING = 50.f;
  float SQUARE_SIZE = 100.f;
  bool showPromotionSquare;
  std::vector<pii> move;
  std::vector<Button> buttons;

  Game game;
  int move_counter;

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
  }

  sf::RectangleShape createSquare(float x, float y, sf::Color c) {
    sf::RectangleShape rect({SQUARE_SIZE, SQUARE_SIZE});
    rect.setFillColor(c);
    rect.setPosition({x, y});
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(0, 0, 0));

    return rect;
  }

  void drawBoard(sf::RenderWindow &window) {
    sf::RectangleShape rect({WIDTH, HEIGHT});
    rect.setFillColor(sf::Color(255, 255, 255));
    rect.setPosition({0, 0});
    window.draw(rect);

    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        sf::Color c(255, 255, 255);
        if((i + j)%2 == 1) c = sf::Color(0, 150, 0);
        window.draw(createSquare(buttons[i * 8 + j].x0, buttons[i * 8 + j].y0, c));
      }
    }

    pii cell = {-1, -1};
    if(move.size() > 0) cell = move[0];

    std::vector<std::pair<pii, int>> specialCells = game.getSpecialCells(cell);
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
      }

      window.draw(createSquare(buttons[x * 8 + y].x0, buttons[x * 8 + y].y0, c));
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

  void drawPiece(sf::RenderWindow &window, std::string piece, float x, float y) {
    sf::Texture texture;
    std::string path = "assets/pieces/" + piece + ".png";
    if(!texture.loadFromFile(path.c_str())) {
      std::cerr << "Failed to open: " << path << "\n";
    }
    sf::Sprite sprite(texture);
    sprite.setScale({0.7f, 0.7f});

    sprite.setPosition({x, y});
    window.draw(sprite);
  }

  void drawPieces(sf::RenderWindow &window) {
    const std::vector<std::vector<std::string>> &setup = game.getBoard(move_counter);
    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        if(setup[i][j] == "") continue;
        drawPiece(window, setup[i][j], PADDING + i * SQUARE_SIZE, PADDING + j * SQUARE_SIZE);
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

  void handlePromotion(int button_id) {
    assert(move.size() == 2);
    game.doAction(move[0], move[1], button_id - 64);
    showPromotionSquare = false;
    move_counter++;
    move.clear();
  }

  void handleAction(int button_id) {
    if(buttons[button_id].getName() == "previous-move") {
      move.clear();
      move_counter = std::max(0, move_counter - 1);
    } else if(buttons[button_id].getName() == "next-move") {
      move.clear();
      move_counter = std::min(game.getTotalMoves(), move_counter + 1);
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
          game.doAction(move[0], move[1]); // Executing move
          move.clear();
          move_counter++;
        }
      } else {
        move.clear(); // Canceling move action
      }
    }
  }

public:
  MatchPage() {}

  MatchPage(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    showPromotionSquare = false;
    move_counter = 0;
    createButtons();
  }

  void refresh(sf::RenderWindow &window) {
    /* Refresh the display */
    drawBoard(window);
    drawPieces(window);
    drawActionButtons(window);
    if(showPromotionSquare) drawPromotionOption(window);
  }

  void handleClick(const sf::Event::MouseButtonPressed *event) {
    int mouse_x = event->position.x;
    int mouse_y = event->position.y;

    int button_id = -1;

    for(int i=0;i<buttons.size();i++) {
      //std::cerr << game.getTotalMoves() << " " << move_counter << "\n";
      /*
        The page has 3 main states:
          1) Promotion time
          2) History: Previous moves
          3) basic game move
      */

      // Promotion time: Only click on promotion options are availables
      if(showPromotionSquare) {
        if(buttons[i].getGroup() == "promotion" && buttons[i].isClicked(mouse_x, mouse_y)) {
          handlePromotion(i);
          break;
        }
        continue;
      }

      // Actions buttons are available at any time now
      if(buttons[i].getGroup() == "action" && buttons[i].isClicked(mouse_x, mouse_y)) {
        handleAction(i);
        break;
      }

      // Basic board click
      if(buttons[i].getGroup() == "board" && game.getTotalMoves() == move_counter && buttons[i].isClicked(mouse_x, mouse_y)) {
        handleBoardClick(i);
        break;
      }
    }
  }
};

#endif
