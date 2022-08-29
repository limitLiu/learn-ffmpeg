#include "Utils/header.h"
#include "Utils/spec.h"
#include <fstream>
#include <thread>

namespace fs = std::filesystem;

Player::Recorder::Recorder() { init(); }

[[maybe_unused]] Player::Recorder::Recorder(const std::string &filename) {
  setFilename(filename);
  init();
}

void Player::Recorder::init() {
  auto fmt = av_find_input_format(FMT_NAME);
  if (!fmt) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n",
           "Failed to call av_find_input_format.");
    return;
  }
  const char *device = DEVICE_NAME;
  int ret = avformat_open_input(&ctx_, device, fmt, nullptr);
  if (ret < 0) {
    log_error(ret);
    return;
  }
}
Player::Recorder::~Recorder() { avformat_close_input(&ctx_); }

// ffmpeg -hide_banner -f avfoundation -i :1 out.wav
[[maybe_unused]] void Player::Recorder::record() {
  if (recording_) {
    stop();
    return;
  }
  std::thread writeThread(&Player::Recorder::write, this);
  writeThread.detach();
}

void Player::Recorder::write() {
  if (filename().empty()) {
    return;
  }

  AVPacket pkt;
  std::ofstream file;
  file.open(filename(), std::ios::binary);
  if (!file.is_open()) {
    return;
  }
  recording_ = true;
  int ret;
  while (recording_) {
    ret = av_read_frame(context(), &pkt);
    if (ret == 0) {
      file.write(reinterpret_cast<const char *>(pkt.data), pkt.size);
    } else if (ret == AVERROR(EAGAIN)) {
      continue;
    } else {
      log_error(ret);
      break;
    }
  }
  file.close();

  Spec spec(ctx_);
  Header header(spec);
  fs::path p(filename());
  header.dataSize = file_size(p);
  p.replace_extension("wav");
  pcm2Wav(header, filename(), p.string());
  recording_ = false;
}

void Player::Recorder::stop() { recording_ = false; }

void Player::Recorder::setFilename(const std::string &filename) {
  filename_ = filename;
}

std::string Player::Recorder::filename() const { return filename_; }

AVFormatContext *Player::Recorder::context() { return ctx_; }

void Player::Recorder::pcm2Wav(Header &header, const std::string &pcmFilename,
                               const std::string &wavFilename) {
  std::ifstream pcm(pcmFilename, std::ios::binary);
  if (!pcm.is_open()) {
    return;
  }
  header.chunkSize = header.dataSize + sizeof(Header) - sizeof(header.chunkID) -
                     sizeof(header.chunkSize);

  std::ofstream wav(wavFilename, std::ios::binary);
  if (!wav.is_open()) {
    return;
  }

  wav.write(reinterpret_cast<const char *>(&header), sizeof(Header));
  char buffer[1024];
  size_t size;
  while ((size = pcm.read(buffer, (int)sizeof(buffer)).gcount()) > 0) {
    wav.write(buffer, (int)size);
  }

  wav.close();
  pcm.close();
}
