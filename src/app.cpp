#include "app.h"
#include "Core/image.h"
#include <cstdlib>

Player::App::App() { init(); }

Player::App::~App() { deinit(); }

[[maybe_unused]] Player::App::App(Window *window) {
  setWindow(window);
  init();
}

void Player::App::init() {
  avdevice_register_all();
  av_log_set_level(AV_LOG_ERROR);
  if (window_ == nullptr) {
    window_ = new Window();
  }
  SDL_JoystickEventState(SDL_ENABLE);
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    joystick_ = SDL_JoystickOpen(i);
  }

  if (!recorder_) {
    recorder_ = new Recorder();
  }

  if (!audio_) {
    recorder_->openDevice(AUDIO_DEVICE_NAME);
    audio_ = new Audio(recorder_->context());
    recorder_->closeDevice();
  }
  renderer_ = window_->init();
  running_ = (renderer() != nullptr);
}

void Player::App::setWindow(Window *window) { window_ = window; }

void Player::App::render() {
  ClearWhite();
  SDL_RenderPresent(renderer());
}

void Player::App::handleEvents() {
  if (SDL_WaitEvent(&event_)) {
    if (SDL_QUIT == event_.type) {
      running_ = false;
    } else if (SDL_KEYDOWN == event_.type) {
      handleKeydown();
    } else if (SDL_JOYBUTTONDOWN == event_.type) {
      handleJoystick();
    } else if (SDL_MOUSEBUTTONDOWN == event_.type) {
      handleMouseClick();
    }
  }
}

void Player::App::handleKeydown() {
  switch (event_.key.keysym.sym) {
  case SDLK_ESCAPE:
    running_ = false;
    break;
  case SDLK_k:
    if (audio_) {
      audio_->setFilename("../resources/out.pcm");
      audio_->play();
    }
    break;
  case SDLK_j:
    if (recorder_) {
      //      recorder_->setFilename("../resources/out.pcm");
      //      recorder_->recordAudio();

      recorder_->setFilename("../resources/out.yuv");
      recorder_->recordVideo();
    }
    break;
  case SDLK_h:
    if (recorder_) {
      recorder_->resample();
      Player::Recorder::pcm2AAC();
      Player::Audio::decodeAAC();
    }
    break;
  case SDLK_l:
    if (audio_) {
      audio_->setFilename("../resources/out.wav");
      audio_->playWAV();
    }
    break;
  case SDLK_SPACE:
    if (recorder_) {
      recorder_->setFilename("../resources/out.wav");
      recorder_->recordWAV();
    }
    break;
  case SDLK_a:
    if (renderer()) {
      Image image(renderer());
      image.load("../resources/image.bmp");
      image.render();
    }
    break;
  case SDLK_b:
    if (renderer()) {
      Image image(renderer());
      image.load("../resources/output.yuv", 1728, 2160);
      image.render();
    }
    break;
  default:
    break;
  }
}

void Player::App::handleJoystick() {
  switch (event_.jbutton.button) {
  case BUp:
    printf("up %d\n", BUp);
    break;
  case BDown:
    printf("down %d\n", BDown);
    break;
  case BLeft:
    printf("left %d\n", BLeft);
    break;
  case BRight:
    printf("right %d\n", BRight);
    break;
  case BCross:
    running_ = false;
    break;
  }
}

void Player::App::deinit() {
  if (joystick_ != nullptr) {
    SDL_JoystickClose(joystick_);
  }
  deletePtr(&window_);
  deletePtr(&audio_);
  deletePtr(&recorder_);
  SDL_DestroyRenderer(renderer_);

  std::atexit(SDL_Quit);
}

void Player::App::handleMouseClick() {
  auto btn = event_.button;
  Image image(renderer());
  image.createTexture();
  SDL_SetRenderTarget(renderer(), nullptr);
  int x = btn.x - (image.width() >> 1);
  int y = btn.y - (image.height() >> 1);
  SDL_Rect dst = {x, y, image.width(), image.height()};
  ClearWhite();
  if (SDL_RenderCopy(renderer(), image.texture(), nullptr, &dst)) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", SDL_GetError());
    return;
  }
  SDL_RenderPresent(renderer());
}

SDL_Renderer *Player::App::renderer() { return renderer_; }
