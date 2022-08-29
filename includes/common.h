#ifndef PLAYER_COMMON_H
#define PLAYER_COMMON_H

#include <SDL.h>
#include <fstream>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavdevice/avdevice.h>
#include <libavutil/avutil.h>

#ifdef __cplusplus
};
#endif

#ifdef _WIN32

#define FMT_NAME "dshow"
#define DEVICE_NAME ""

#elif __APPLE__

#define FMT_NAME ("avfoundation")
#define DEVICE_NAME ":1"

#endif

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

using Byte = Uint8;

#endif // PLAYER_COMMON_H
