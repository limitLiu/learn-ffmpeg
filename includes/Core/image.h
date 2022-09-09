#ifndef PLAYER_IMAGE_H
#define PLAYER_IMAGE_H

#include "common.h"

namespace Player {
class Window;

class Image {
public:
  Image() = default;

  ~Image();

  explicit Image(SDL_Renderer *renderer);

  void load(const std::string &filename);

  void load(const std::string &filename, int width, int height,
            uint32_t format = SDL_PIXELFORMAT_IYUV);

  void setWidth(int width);

  [[nodiscard]] int width() const;

  void setHeight(int height);

  [[nodiscard]] int height() const;

  [[nodiscard]] bool fit() const;

  void createTexture();

  void render();

  SDL_Renderer *renderer();

  [[maybe_unused]] void setRenderer(SDL_Renderer *renderer);

  SDL_Texture *texture();

private:
  void deinit();

private:
  SDL_Renderer *renderer_ = nullptr;

  int width_ = 0;

  int height_ = 0;

  SDL_Texture *texture_ = nullptr;
};
} // namespace Player

#endif // PLAYER_IMAGE_H
