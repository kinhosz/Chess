#ifndef MATCHPAGE_HPP
#define MATCHPAGE_HPP

#include <SFML/Graphics.hpp>

#include <Game.hpp>

class MatchPage {
private:
  int WIDTH, HEIGHT;
  float PADDING = 50.f;
  float SQUARE_SIZE = 100.f;

  Game game;

  sf::RectangleShape createSquare(int x, int y, sf::Color c) {
    sf::RectangleShape rect({SQUARE_SIZE, SQUARE_SIZE});
    rect.setFillColor(c);
    rect.setPosition(
      {PADDING + SQUARE_SIZE * x, PADDING + SQUARE_SIZE * y}
    );
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(0, 0, 0));

    return rect;
  }

  void drawBoard(sf::RenderWindow &window) {
    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        sf::Color c(255, 255, 255);
        if((i + j)%2 == 1) c = sf::Color(0, 150, 0);
        window.draw(createSquare(i, j, c));
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

      window.draw(createSquare(x, y, c));
    }
  }

  void drawPiece(sf::RenderWindow &window, std::string piece, int x, int y) {
    sf::Texture texture;
    std::string path = "assets/" + piece + ".png";
    if(!texture.loadFromFile(path.c_str())) {
      std::cerr << "Failed to open: " << path << "\n";
    }
    sf::Sprite sprite(texture);
    sprite.setScale({0.7f, 0.7f});

    sprite.setPosition({PADDING + x * SQUARE_SIZE, PADDING + y * SQUARE_SIZE});
    window.draw(sprite);
  }

  void drawPieces(sf::RenderWindow &window) {
    const std::vector<std::vector<std::string>> &setup = game.getBoard();
    for(int i=0;i<8;i++) {
      for(int j=0;j<8;j++) {
        if(setup[i][j] == "") continue;
        drawPiece(window, setup[i][j], i, j);
      }
    }
  }

public:
  MatchPage() {}

  MatchPage(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
  }

  void refresh(sf::RenderWindow &window) {
    /* Refresh the display */
    drawBoard(window);
    drawPieces(window);
  }

  void handleClick(const sf::Event::MouseButtonPressed *event) {
    int x = event->position.x;
    int y = event->position.y;

    x -= PADDING;
    y -= PADDING;
    x /= SQUARE_SIZE;
    y /= SQUARE_SIZE;
    if(x < 0 || x > 7 || y < 0 || y > 7) return;
    game.handleClickOnCell({x, y});
  }
};

#endif
