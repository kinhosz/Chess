#ifndef MATCHPAGE_HPP
#define MATCHPAGE_HPP

#include <SFML/Graphics.hpp>

#include <Game.hpp>

class Button {
  std::string group = "";
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
  std::vector<Button> buttons;

  Game game;

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
    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        sf::Color c(255, 255, 255);
        if((i + j)%2 == 1) c = sf::Color(0, 150, 0);
        window.draw(createSquare(buttons[i * 8 + j].x0, buttons[i * 8 + j].y0, c));
      }
    }

    std::vector<std::pair<pii, int>> specialCells = game.getSpecialCells();
    for(int i=0;i<specialCells.size();i++) {
      int x = specialCells[i].first.first;
      int y = specialCells[i].first.second;
      int info = specialCells[i].second;

      sf::Color c(218, 154, 44); // Piece moved
      if(info == 1) {
        c = sf::Color(200, 0, 0); // CheckMate
      } else if(info == -1) {
        c = sf::Color(100, 100, 100); // Draw
      }

      window.draw(createSquare(buttons[x * 8 + y].x0, buttons[x * 8 + y].y0, c));
    }
  }

  void drawPiece(sf::RenderWindow &window, std::string piece, float x, float y) {
    sf::Texture texture;
    std::string path = "assets/" + piece + ".png";
    if(!texture.loadFromFile(path.c_str())) {
      std::cerr << "Failed to open: " << path << "\n";
    }
    sf::Sprite sprite(texture);
    sprite.setScale({0.7f, 0.7f});

    sprite.setPosition({x, y});
    window.draw(sprite);
  }

  void drawPieces(sf::RenderWindow &window) {
    const std::vector<std::vector<std::string>> &setup = game.getBoard();
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

public:
  MatchPage() {}

  MatchPage(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    showPromotionSquare = false;
    createButtons();
  }

  void refresh(sf::RenderWindow &window) {
    /* Refresh the display */
    drawBoard(window);
    drawPieces(window);
    if(showPromotionSquare) drawPromotionOption(window);
  }

  void handleClick(const sf::Event::MouseButtonPressed *event) {
    int x = event->position.x;
    int y = event->position.y;

    int button_id = -1;

    for(int i=0;i<buttons.size();i++) {
      if(buttons[i].getGroup() == "board") {
        if(showPromotionSquare) continue;
        if(buttons[i].isClicked(x, y)) button_id = i; 
      } else if(buttons[i].getGroup() == "promotion") {
        if(!showPromotionSquare) continue;
        if(buttons[i].isClicked(x, y)) button_id = i;
      }
    }

    if(button_id == -1) return;
    int info = game.handleClickOnCell(button_id);
    if(info == 1) showPromotionSquare = true;
    else showPromotionSquare = false;
  }
};

#endif
