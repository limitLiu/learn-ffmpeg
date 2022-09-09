#ifndef PLAYER_WINDOW_H
#define PLAYER_WINDOW_H

#include "common.h"

namespace Player {

class Image;

class Window {
public:
  Window() = default;

  ~Window();

  SDL_Renderer *init();

private:
  void deinit();

private:
  SDL_Window *window_ = nullptr;
};

} // namespace Player

#endif // PLAYER_WINDOW_H
