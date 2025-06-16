#include <Game.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Event.hpp>

const float PADDING = 50.f;
const float SIDE = 100.f;

void drawBoard(sf::RenderWindow &window, const Game &game) {
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      sf::RectangleShape rect({SIDE, SIDE});
      sf::Color c(255, 255, 255);
      if((i+j)%2 == 1) c = sf::Color(0, 150, 0);
      rect.setFillColor(c);
      rect.setPosition({PADDING + float(SIDE * i), PADDING + float(SIDE * j)});
      rect.setOutlineThickness(1.f);
      rect.setOutlineColor(sf::Color(0, 0, 0));
      window.draw(rect);
    }
  }

  const std::vector<pii> markedCells = game.getMarkedCells();
  for(pii cell: markedCells) {
    sf::Color c(218, 154, 44);
    sf::RectangleShape rect({SIDE, SIDE});
    rect.setFillColor(c);
    rect.setPosition({PADDING + float(SIDE * cell.first), PADDING + float(SIDE * cell.second)});
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(0, 0, 0));
    window.draw(rect);
  }

  if(game.isDraw()) {
    pii king = game.getKingPos();
    sf::Color c(100, 100, 100);
    sf::RectangleShape rect({SIDE, SIDE});
    rect.setFillColor(c);
    rect.setPosition({PADDING + float(SIDE * king.first), PADDING + float(SIDE * king.second)});
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(0, 0, 0));
    window.draw(rect);
  }
  if(game.isCheckMate()) {
    pii king = game.getKingPos();
    sf::Color c(200, 0, 0);
    sf::RectangleShape rect({SIDE, SIDE});
    rect.setFillColor(c);
    rect.setPosition({PADDING + float(SIDE * king.first), PADDING + float(SIDE * king.second)});
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(0, 0, 0));
    window.draw(rect);
  }
}

void drawPiece(sf::RenderWindow &window, std::string piece, pii cell) {
  sf::Texture texture;
  std::string path = "assets/" + piece + ".png";
  if(!texture.loadFromFile(path.c_str())) {
    std::cerr << "Erro ao abrir " << path << "\n";
  }
  sf::Sprite sprite(texture);
  sprite.setScale({0.7f, 0.7f});

  sprite.setPosition({PADDING + cell.first * SIDE, PADDING + cell.second * SIDE});
  window.draw(sprite);
}

void drawPieces(sf::RenderWindow &window, const std::vector<std::vector<std::string>> &setup) {
  for(int i=0;i<8;i++) {
    for(int j=0;j<8;j++) {
      if(setup[i][j] == "") continue;
      drawPiece(window, setup[i][j], {i, j});
    }
  }
}

pii handleMouseClick(const sf::Event::MouseButtonPressed *event) {
  int x = event->position.x;
  int y = event->position.y;

  x -= PADDING;
  y -= PADDING;

  x /= SIDE;
  y /= SIDE;

  if(x < 0 || x > 7 || y < 0 || y > 7) return {-1, -1};
  return {x, y};
}

int main() {
  int WIDTH = 1200;
  int HEIGHT = 900;
  sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Chess");
  window.setPosition({-1, 0});

  Game game;

  while(window.isOpen()) {
    while(const std::optional event = window.pollEvent()) {
      if(event->is<sf::Event::Closed>()) {
        window.close();
      } else if(event->is<sf::Event::MouseButtonPressed>()) {
        pii cell = handleMouseClick(event->getIf<sf::Event::MouseButtonPressed>());
        if(cell.first == -1) continue;
        game.handleClickOnCell(cell);
      }
    }

    window.clear();

    drawBoard(window, game);
    drawPieces(window, game.getBoard());

    window.display();
  }
}
