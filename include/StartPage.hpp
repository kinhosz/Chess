#ifndef STARTPAGE_HPP
#define STARTPAGE_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Font.hpp>

#include <Button.hpp>

// Match modes, mirroring MatchPage's MATCH_MODE bit encoding:
// bit0 (1) set -> black is bot-controlled, bit1 (2) set -> white is bot-controlled.
#define MODE_TWO_PLAYERS 0
#define MODE_PLAY_WHITE 1
#define MODE_PLAY_BLACK 2
#define MODE_SELF_PLAY 3

class StartPage {
private:
  int WIDTH, HEIGHT;
  std::vector<Button> depthButtons;
  std::vector<Button> modeButtons;
  std::vector<int> depthValues = {3, 4, 5, 6, 7, 8};
  std::vector<std::pair<std::string,int>> modes = {
    {"Self-play (bot x bot)", MODE_SELF_PLAY},
    {"2 players", MODE_TWO_PLAYERS},
    {"Play as White", MODE_PLAY_WHITE},
    {"Play as Black", MODE_PLAY_BLACK},
  };

  int selectedDepth = 5;
  bool started = false;
  int chosenMode = MODE_SELF_PLAY;

  sf::RectangleShape createBox(const Button &b, sf::Color c) {
    sf::RectangleShape rect({b.xf - b.x0, b.yf - b.y0});
    rect.setFillColor(c);
    rect.setPosition({b.x0, b.y0});
    rect.setOutlineThickness(1.f);
    rect.setOutlineColor(sf::Color(0, 0, 0));
    return rect;
  }

  void drawLabel(sf::RenderWindow &window, const std::string &label, float x, float y, unsigned int size=18) {
    sf::Font font("assets/fonts/bitcount_prop_single.ttf");
    sf::Text text(font);
    text.setString(label);
    text.setCharacterSize(size);
    text.setFillColor(sf::Color(225, 225, 230));
    text.setPosition({x, y});
    window.draw(text);
  }

public:
  StartPage() {}

  StartPage(int width, int height) {
    WIDTH = width;
    HEIGHT = height;

    float depthBtnW = 60.f, depthBtnH = 50.f, depthGap = 15.f;
    float depthTotalW = depthValues.size() * depthBtnW + (depthValues.size() - 1) * depthGap;
    float depthX0 = (WIDTH - depthTotalW) / 2.f;
    float depthY0 = 260.f;
    for(size_t i=0;i<depthValues.size();i++) {
      float x0 = depthX0 + i * (depthBtnW + depthGap);
      depthButtons.push_back(Button(x0, x0 + depthBtnW, depthY0, depthY0 + depthBtnH));
    }

    float modeBtnW = 320.f, modeBtnH = 60.f, modeGap = 20.f;
    float modeX0 = (WIDTH - modeBtnW) / 2.f;
    float modeY0 = 420.f;
    for(size_t i=0;i<modes.size();i++) {
      float y0 = modeY0 + i * (modeBtnH + modeGap);
      modeButtons.push_back(Button(modeX0, modeX0 + modeBtnW, y0, y0 + modeBtnH));
    }
  }

  bool isStarted() const { return started; }
  int getDepth() const { return selectedDepth; }
  int getMode() const { return chosenMode; }

  void refresh(sf::RenderWindow &window) {
    sf::RectangleShape bg({(float)WIDTH, (float)HEIGHT});
    bg.setFillColor(sf::Color(24, 26, 33));
    bg.setPosition({0, 0});
    window.draw(bg);

    drawLabel(window, "Chess", WIDTH / 2.f - 40, 60, 36);
    drawLabel(window, "Search depth:", WIDTH / 2.f - 200, 220, 20);

    for(size_t i=0;i<depthButtons.size();i++) {
      bool selected = depthValues[i] == selectedDepth;
      sf::Color c = selected ? sf::Color(0, 150, 0) : sf::Color(70, 70, 80);
      window.draw(createBox(depthButtons[i], c));
      drawLabel(window, std::to_string(depthValues[i]),
                depthButtons[i].x0 + 22, depthButtons[i].y0 + 12);
    }

    drawLabel(window, "Mode (click to start):", WIDTH / 2.f - 200, 380, 20);
    for(size_t i=0;i<modeButtons.size();i++) {
      window.draw(createBox(modeButtons[i], sf::Color(70, 70, 80)));
      drawLabel(window, modes[i].first, modeButtons[i].x0 + 20, modeButtons[i].y0 + 18);
    }
  }

  void handleClick(const sf::Event::MouseButtonPressed *event) {
    int mx = event->position.x;
    int my = event->position.y;

    for(size_t i=0;i<depthButtons.size();i++) {
      if(depthButtons[i].isClicked(mx, my)) {
        selectedDepth = depthValues[i];
        return;
      }
    }

    for(size_t i=0;i<modeButtons.size();i++) {
      if(modeButtons[i].isClicked(mx, my)) {
        chosenMode = modes[i].second;
        started = true;
        return;
      }
    }
  }
};

#endif
