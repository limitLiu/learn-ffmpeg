#ifndef PLAYER_COMMON_H
#define PLAYER_COMMON_H

#include <SDL.h>
#include <SDL_image.h>
#include <fstream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>

#ifdef __cplusplus
};
#endif

#ifdef _WIN32

#define FMT_NAME "dshow"
#define DEVICE_NAME ""

#elif __APPLE__

#define FMT_NAME ("avfoundation")
#define AUDIO_DEVICE_NAME ":1"
#define VIDEO_DEVICE_NAME "0:"

#endif

#define ClearWhite() ClearWindow(255, 255, 255)

#define ClearWindow(R, G, B)                                                                       \
  if (SDL_SetRenderDrawColor(renderer(), R, G, B, SDL_ALPHA_OPAQUE)) {                       \
    av_log(nullptr, AV_LOG_ERROR, "%s\n", SDL_GetError());                                         \
    return;                                                                                        \
  }                                                                                                \
  if (SDL_RenderClear(renderer())) {                                                               \
    av_log(nullptr, AV_LOG_ERROR, "%s\n", SDL_GetError());                                         \
    return;                                                                                        \
  }

enum {
  WIDTH = 960,
  HEIGHT = 544,
};

template <typename T> void deletePtr(T **ptr) {
  if (ptr != nullptr && *ptr != nullptr) {
    delete *ptr;
    *ptr = nullptr;
  }
}

static void log_error(int code) {
  char buf[1024];
  av_strerror(code, buf, sizeof(buf));
  av_log(nullptr, AV_LOG_ERROR, "%s\n", buf);
}

using Byte = uint8_t;

#endif // PLAYER_COMMON_H
