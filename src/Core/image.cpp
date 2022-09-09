#include "Core/image.h"
#include "GUI/window.h"

#include <filesystem>

namespace fs = std::filesystem;

Player::Image::~Image() { deinit(); }

Player::Image::Image(SDL_Renderer *renderer) { renderer_ = renderer; }

void Player::Image::deinit() {
  if (texture_) {
    SDL_DestroyTexture(texture_);
  }
}

void Player::Image::load(const std::string &filename) {
  if (filename.empty()) {
    av_log(nullptr, AV_LOG_ERROR, "image file isn't set\n");
    return;
  }
  auto surface = IMG_Load(filename.c_str());
  if (!surface) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to load picture\n");
    return;
  }
  texture_ = SDL_CreateTextureFromSurface(renderer(), surface);
  if (!texture_) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", SDL_GetError());
    return;
  }
  setWidth(surface->w);
  setHeight(surface->h);
  SDL_FreeSurface(surface);
}

void Player::Image::setWidth(int width) { width_ = width; }

int Player::Image::width() const { return width_; }

void Player::Image::setHeight(int height) { height_ = height; }

int Player::Image::height() const { return height_; }

bool Player::Image::fit() const { return width() <= height(); }

void Player::Image::createTexture() {
  auto texture =
      SDL_CreateTexture(renderer(), SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_TARGET, 50, 50);
  if (!texture) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to create surface\n");
    return;
  }
  if (SDL_SetRenderTarget(renderer(), texture)) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", SDL_GetError());
    return;
  }

  if (SDL_SetRenderDrawColor(renderer(), 100, 100, 0, SDL_ALPHA_OPAQUE)) {
    return;
  }

  SDL_Rect rect = {0, 0, 50, 50};
  if (SDL_RenderDrawRect(renderer(), &rect)) {
    return;
  }

  if (SDL_RenderDrawLine(renderer(), 0, 0, 50, 50)) {
    return;
  }

  if (SDL_RenderDrawLine(renderer(), 50, 0, 0, 50)) {
    return;
  }

  if (SDL_RenderDrawLine(renderer(), 25, 0, 25, 50)) {
    return;
  }

  if (SDL_RenderDrawLine(renderer(), 0, 25, 50, 25)) {
    return;
  }

  setWidth(50);
  setHeight(50);
  texture_ = texture;
}

void Player::Image::render() {
  ClearWhite();
  SDL_Rect dstRect = {0, 0, WIDTH, HEIGHT};
  if (fit()) {
    auto w = (width() * HEIGHT * 1.0) / height() * 1.0;
    auto x = (WIDTH * 1.0 - w) / 2;
    dstRect.x = static_cast<int>(x);
    dstRect.w = static_cast<int>(w);
  }
  SDL_RenderCopyEx(renderer(), texture_, nullptr, &dstRect, 0, nullptr, SDL_FLIP_NONE);
  SDL_RenderPresent(renderer());
}

SDL_Renderer *Player::Image::renderer() { return renderer_; }

[[maybe_unused]] void Player::Image::setRenderer(SDL_Renderer *renderer) { renderer_ = renderer; }

SDL_Texture *Player::Image::texture() { return texture_; }

void Player::Image::load(const std::string &filename, int width, int height, uint32_t format) {
  if (filename.empty()) {
    av_log(nullptr, AV_LOG_ERROR, "image file isn't set\n");
    return;
  }
  std::ifstream input;
  input.open(filename, std::ios::binary);
  if (!input.is_open()) {
    av_log(nullptr, AV_LOG_ERROR, "Failed to open file\n");
    return;
  }
  texture_ = SDL_CreateTexture(renderer(), format, SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!texture_) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", SDL_GetError());
    return;
  }
  //  int len = (int) input.seekg(0, std::ios::end).tellg();
  fs::path p(filename);
  int len = (int)file_size(p);
  auto buffer = new char[len];

  input.read(buffer, len);
  if (SDL_UpdateTexture(texture_, nullptr, buffer, width)) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", SDL_GetError());
    return;
  }
  setWidth(width);
  setHeight(height);
}
