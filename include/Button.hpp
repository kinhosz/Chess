#ifndef BUTTON_HPP
#define BUTTON_HPP

#include <string>

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

#endif
