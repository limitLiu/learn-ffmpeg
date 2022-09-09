#include "GUI/window.h"

SDL_Renderer *Player::Window::init() {
  SDL_Renderer *renderer = nullptr;
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) >= 0) {
    window_ = SDL_CreateWindow("Media Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window_ != nullptr) {
      renderer =
          SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      if (!renderer) {
        renderer = SDL_CreateRenderer(window_, -1, 0);
      }
    }
  }
  return renderer;
}

Player::Window::~Window() { deinit(); }

void Player::Window::deinit() {
  SDL_DestroyWindow(window_);
  window_ = nullptr;
}
