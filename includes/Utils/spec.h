#ifndef PLAYER_SPEC_H
#define PLAYER_SPEC_H

#include "Core/audio.h"
#include "common.h"

namespace Player {
struct Spec {
  int sampleRate = 48000;
  int bitsPerSample = 32;
  int channels = 1;
  // 音频缓冲区样本数量
  int samples = 1024;
  AVCodecID codecID = AV_CODEC_ID_NONE;

  Spec() = default;

  explicit Spec(AVFormatContext *ctx);

  ~Spec() = default;

  void setCodecID(AVCodecID codecId);
};

struct ResampleAudioSpec {
  std::string filename;
  int sampleRate;
  AVSampleFormat fmt;
  AVChannelLayout channelLayout;
};
} // namespace Player

#endif // PLAYER_SPEC_H
