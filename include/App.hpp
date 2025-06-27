#ifndef APP_HPP
#define APP_HPP

#include <string>
#include <SFML/Graphics.hpp>

#include <MatchPage.hpp>

class App {
private:
  int WIDTH, HEIGHT;
  sf::RenderWindow window;
  MatchPage matchPage;

  void updateDisplay() {
    window.clear();
    matchPage.refresh(window);
    window.display();
  }

  void handleEventQueue() {
    while(const std::optional event = window.pollEvent()) {
      if(event->is<sf::Event::Closed>()) window.close();
      else if(event->is<sf::Event::MouseButtonPressed>()) {
        matchPage.handleClick(
          event->getIf<sf::Event::MouseButtonPressed>()
        );
      }
    }
  }

public:
  App(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    window = sf::RenderWindow(sf::VideoMode({width, height}), "chess");
    matchPage = MatchPage(WIDTH, HEIGHT);
  }

  void run() {
    while(window.isOpen()) {
      handleEventQueue();
      updateDisplay();
      sf::sleep(sf::seconds(1));
    }
  }
};

#endif
