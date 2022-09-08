#ifndef PLAYER_RECORDER_H
#define PLAYER_RECORDER_H

#include "common.h"

namespace Player {
struct Header;
struct ResampleAudioSpec;

class Recorder {

public:
  enum AudioFmt {
    FmtS16 = AV_SAMPLE_FMT_S16,
    FmtFlt = AV_SAMPLE_FMT_FLT,
  };

  Recorder() = default;

  [[maybe_unused]] explicit Recorder(const std::string &filename);

  ~Recorder() = default;

  [[maybe_unused]] void recordAudio();

  void recordVideo();

  void recordWAV();

  void stop();

  void setFilename(const std::string &filename);

  [[nodiscard]] std::string filename() const;

  AVFormatContext *context();

  static void pcm2Wav(Header &header, const std::string &pcmFilename,
                      const std::string &wavFilename);

  void resample() const;

  static void resample(const std::string &inputName, int inputSampleRate, AVSampleFormat inputFmt,
                       AVChannelLayout inputChLayout, const std::string &outputName,
                       int outputSampleRate, AVSampleFormat outputFmt,
                       AVChannelLayout outputChLayout);

  static void resample(ResampleAudioSpec &input, ResampleAudioSpec &output);

  static void pcm2AAC(ResampleAudioSpec &spec, std::string aacFilename = "");

  static void pcm2AAC();

  bool openDevice(const char *device, AVDictionary **opts = nullptr);

  void closeDevice();

private:
  void writePCM();

  void writeYUV();

  void writeWAV();

  static bool checkSampleFmt(const AVCodec *codec, AVSampleFormat fmt);

  static int encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt, std::ofstream &output);

private:
  std::string filename_;

  std::atomic<bool> recording_{false};

  AVFormatContext *ctx_ = nullptr;

#ifdef _WIN32
  AudioFmt fmt_ = FmtS16;
#elif __APPLE__
  AudioFmt fmt_ = FmtFlt;
#endif
};

} // namespace Player

#endif // PLAYER_RECORDER_H
