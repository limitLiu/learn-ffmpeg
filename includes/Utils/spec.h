#ifndef PLAYER_SPEC_H
#define PLAYER_SPEC_H

#include "Core/audio.h"
#include "common.h"

namespace Player {
struct Spec {
  int sampleRate = 44100;
  int bitsPerSample = 32;
  int channels = 2;
  // 音频缓冲区样本数量
  int samples = 1024;
  AVCodecID codecID;

  Spec() = default;

  explicit Spec(AVFormatContext *ctx);

  ~Spec() = default;
};
} // namespace Player

#endif // PLAYER_SPEC_H
