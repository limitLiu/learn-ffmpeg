#include "GUI/window.h"

Player::Window::Window(bool isInit) {
  if (isInit) {
    inited = init();
  }
}

bool Player::Window::init() {
  auto running = false;
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) >= 0) {
    window_ = SDL_CreateWindow("Media Player", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
                               SDL_WINDOW_SHOWN);
    if (window_ != nullptr) {
      renderer_ = SDL_CreateRenderer(
          window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    }
    running = true;
  }
  return running;
}

Player::Window::~Window() {
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  window_ = nullptr;
  renderer_ = nullptr;
  inited = false;
}

void Player::Window::render() {
  SDL_RenderClear(renderer_);
  SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
  SDL_RenderPresent(renderer_);
}
