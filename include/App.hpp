#ifndef APP_HPP
#define APP_HPP

#include <string>
#include <SFML/Graphics.hpp>

#include <MatchPage.hpp>
#include <StartPage.hpp>

class App {
private:
  int WIDTH, HEIGHT;
  sf::RenderWindow window;
  StartPage startPage;
  MatchPage matchPage;
  bool matchStarted = false;

  void updateDisplay() {
    window.clear();
    if(matchStarted) matchPage.refresh(window);
    else startPage.refresh(window);
    window.display();
  }

  void handleEventQueue() {
    while(const std::optional event = window.pollEvent()) {
      if(event->is<sf::Event::Closed>()) window.close();
      else if(event->is<sf::Event::MouseButtonPressed>()) {
        const auto *click = event->getIf<sf::Event::MouseButtonPressed>();
        if(matchStarted) matchPage.handleClick(click);
        else {
          startPage.handleClick(click);
          if(startPage.isStarted()) {
            matchPage = MatchPage(WIDTH, HEIGHT, startPage.getMode(), startPage.getDepth());
            matchStarted = true;
          }
        }
      }
    }
  }

public:
  App(int width, int height) {
    WIDTH = width;
    HEIGHT = height;
    window = sf::RenderWindow(sf::VideoMode({width, height}), "chess");
    startPage = StartPage(WIDTH, HEIGHT);
  }

  void run() {
    while(window.isOpen()) {
      handleEventQueue();
      updateDisplay();
    }
  }
};

#endif
