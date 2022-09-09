#ifndef PLAYER_APP_H
#define PLAYER_APP_H

#include "Core/audio.h"
#include "Core/recorder.h"
#include "GUI/window.h"

namespace Player {

enum CTRL {
  BTriangle = 0,
  BCircle = 1,
  BCross = 2,
  BSquare = 3,

  BL = 4,
  BR = 5,

  BDown = 6,
  BLeft = 7,
  BUp = 8,
  BRight = 9,

  BSelect = 10,
  BStart = 11
};

class App {
public:
  App();

  [[maybe_unused]] explicit App(Window *window);

  ~App();

  void setWindow(Window *window);

  void handleEvents();

  [[nodiscard]] bool running() const { return running_; }

  SDL_Renderer *renderer();

  void render();

private:
  void init();

  void handleKeydown();

  void handleJoystick();

  void handleMouseClick();

  void deinit();

private:
  Window *window_ = nullptr;

  Audio *audio_ = nullptr;

  Recorder *recorder_ = nullptr;

  bool running_ = false;

  SDL_Joystick *joystick_ = nullptr;

  SDL_Event event_{};

  SDL_Renderer *renderer_ = nullptr;
};

} // namespace Player

#endif // PLAYER_APP_H
