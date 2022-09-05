#ifndef PLAYER_AUDIO_H
#define PLAYER_AUDIO_H

#include "common.h"

namespace Player {
struct Spec;
struct ResampleAudioSpec;

struct AudioBuffer {
  size_t len = 0;
  size_t pullSize = 0;
  Byte *data = nullptr;
};

class Audio {
public:
  enum AudioFormat {
    FormatU8 [[maybe_unused]] = AUDIO_U8,
    FormatS8 [[maybe_unused]] = AUDIO_S8,
    FormatU16LSB = AUDIO_U16LSB,
    FormatS16LSB = AUDIO_S16LSB,
    FormatU16MSB [[maybe_unused]] = AUDIO_U16MSB,
    FormatS16MSB [[maybe_unused]] = AUDIO_S16MSB,
    FormatU16 [[maybe_unused]] = FormatU16LSB,
    FormatS16 = FormatS16LSB,
    FormatF32LSB = AUDIO_F32LSB,
    FormatF32MSB [[maybe_unused]] = AUDIO_F32MSB,
  };

  enum AudioFmt {
    FmtS16 = AV_SAMPLE_FMT_S16,
    FmtFlt = AV_SAMPLE_FMT_FLT,
  };

  Audio();

  [[maybe_unused]] explicit Audio(const std::string &name);

  ~Audio();

  [[maybe_unused]] explicit Audio(const std::string &name,
                                  AVFormatContext *ctx);

  [[maybe_unused]] explicit Audio(AVFormatContext *ctx);

  void setFilename(const std::string &name);

  [[nodiscard]] std::string filename() const;

  void play();

  void stop();

  void playWAV();

  [[maybe_unused]] void setSampleRate(int sampleRate) const;
  [[nodiscard]] int sampleRate() const;

  [[maybe_unused]] void setFormat(AudioFormat format);
  [[nodiscard]] AudioFormat format() const;

  [[maybe_unused]] void setChannels(int channels) const;
  [[nodiscard]] int channels() const;

  [[maybe_unused]] void setSamples(int samples) const;
  [[nodiscard]] int samples() const;

  [[nodiscard]] Spec *spec() const;

  void resample() const;

  static void resample(const std::string &inputName, int inputSampleRate,
                       AVSampleFormat inputFmt, AVChannelLayout inputChLayout,
                       const std::string &outputName, int outputSampleRate,
                       AVSampleFormat outputFmt,
                       AVChannelLayout outputChLayout);

  static void resample(ResampleAudioSpec &input, ResampleAudioSpec &output);

private:
  void run();

  void runWAV();

  bool parseWAV(SDL_AudioSpec &spec, std::ifstream &input) const;

  [[nodiscard]] int bufferSize();

  [[nodiscard]] int bytesPerSample() const;

  void init();

  void initWithFormatContext(AVFormatContext *ctx);

private:
  std::string filename_;

  Spec *spec_ = nullptr;

  std::atomic<bool> playing_{false};

  static SDL_AudioFormat getSDLFormat(uint16_t audioFormat,
                                      uint16_t bitsPerSample, bool &success);

#ifdef _WIN32
  AudioFormat format_ = FormatS16;
  AudioFmt fmt_ = FmtS16;
#elif __APPLE__
  AudioFormat format_ = FormatF32LSB;
  AudioFmt fmt_ = FmtFlt;
#endif
};

} // namespace Player

#endif // PLAYER_AUDIO_H
