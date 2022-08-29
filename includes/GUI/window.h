#ifndef PLAYER_WINDOW_H
#define PLAYER_WINDOW_H

#include "common.h"

namespace Player {

enum {
  WIDTH = 960,
  HEIGHT = 544,
};

class Window {
public:
  explicit Window(bool isInit = true);

  ~Window();

  void render();

  bool init();

public:
  bool inited = false;

private:
  SDL_Window *window_ = nullptr;

  SDL_Renderer *renderer_ = nullptr;
};

} // namespace Player

#endif // PLAYER_WINDOW_H
