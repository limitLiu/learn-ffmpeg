#include "app.h"
#include <cstdlib>

Player::App::App() { init(); }

Player::App::~App() { deinit(); }

Player::App::App(Window *window) {
  setWindow(window);
  init();
}

void Player::App::init() {
  avdevice_register_all();
  av_log_set_level(AV_LOG_ERROR);
  if (window_ == nullptr) {
    window_ = new Window();
  } else if (!window_->inited) {
    window_->inited = window_->init();
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
  running_ = window_->inited;
}

void Player::App::setWindow(Window *window) { window_ = window; }

void Player::App::render() { window_->render(); }

void Player::App::handleEvents() {
  if (SDL_PollEvent(&event_)) {
    if (SDL_QUIT == event_.type) {
      running_ = false;
    } else if (SDL_KEYDOWN == event_.type) {
      handleKeydown();
    } else if (SDL_JOYBUTTONDOWN == event_.type) {
      handleJoystick();
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

  std::atexit(SDL_Quit);
}
