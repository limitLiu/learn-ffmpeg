#ifndef PLAYER_RECORDER_H
#define PLAYER_RECORDER_H

#include "common.h"

namespace Player {
struct Header;
class Recorder {
public:
  Recorder();

  [[maybe_unused]] explicit Recorder(const std::string &filename);

  ~Recorder();

  [[maybe_unused]] void record();

  void recordWAV();

  void stop();

  void setFilename(const std::string &filename);

  [[nodiscard]] std::string filename() const;

  AVFormatContext *context();

  static void pcm2Wav(Header &header, const std::string &pcmFilename,
                      const std::string &wavFilename);

private:
  void write();

  void init();

  void deinit();

  void writeWAV();

private:
  std::string filename_;

  std::atomic<bool> recording_{false};

  AVFormatContext *ctx_ = nullptr;
};

} // namespace Player

#endif // PLAYER_RECORDER_H
