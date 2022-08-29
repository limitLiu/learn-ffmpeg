#include "Core/audio.h"
#include "Utils/spec.h"

#include <thread>

std::atomic<bool> audioRunning(true);

Player::Audio::Audio() { init(); }

Player::Audio::~Audio() { deletePtr(&spec_); }

[[maybe_unused]] Player::Audio::Audio(const std::string &name) {
  setFilename(name);
  init();
}

[[maybe_unused]] Player::Audio::Audio(const std::string &name,
                                      AVFormatContext *ctx) {
  setFilename(name);
  initWithFormatContext(ctx);
}

Player::Audio::Audio(AVFormatContext *ctx) { initWithFormatContext(ctx); }

void Player::Audio::init() { spec_ = new Spec(); }

void Player::Audio::initWithFormatContext(AVFormatContext *ctx) {
  spec_ = new Spec(ctx);
}

std::string Player::Audio::filename() const { return filename_; }

void Player::Audio::setFilename(const std::string &name) { filename_ = name; }

void Player::Audio::play() {
  if (filename().empty()) {
    return;
  }
  std::thread audio(&Player::Audio::run, this);
  audio.detach();
}

void pullAudioData(void *userdata, Byte *stream, int len) {
  SDL_memset(stream, 0, len);
  auto buffer = (Player::AudioBuffer *)userdata;
  if (buffer->len < 1) {
    return;
  }
  buffer->pullSize = len > buffer->len ? buffer->len : len;
  SDL_MixAudio(stream, buffer->data, buffer->pullSize, SDL_MIX_MAXVOLUME);
  buffer->data += buffer->pullSize;
  buffer->len -= buffer->pullSize;
}

void Player::Audio::run() {
  audioRunning = true;
  SDL_AudioSpec spec;
  spec.freq = sampleRate();
  spec.channels = channels();
  spec.format = format();
  spec.samples = samples();
  spec.callback = pullAudioData;
  AudioBuffer audioBuffer;
  spec.userdata = &audioBuffer;

  if (SDL_OpenAudio(&spec, nullptr)) {
    return;
  }

  std::ifstream input;
  input.open(filename(), std::ios::binary);
  if (!input.is_open()) {
    SDL_CloseAudio();
    return;
  }

  SDL_PauseAudio(0);

  Byte buffer[bufferSize()];
  while (audioRunning) {
    if (audioBuffer.len) {
      continue;
    }
    audioBuffer.len =
        input.read(reinterpret_cast<char *>(buffer), bufferSize()).gcount();
    if (audioBuffer.len < 1) {
      auto ms = audioBuffer.pullSize / bytesPerSample() / sampleRate();
      SDL_Delay(ms * 1000);
      break;
    }
    audioBuffer.data = buffer;
  }

  input.close();
  SDL_CloseAudio();
  audioRunning = false;
}

int Player::Audio::sampleRate() const { return spec()->sampleRate; }

Player::Audio::AudioFormat Player::Audio::format() const { return format_; }

int Player::Audio::channels() const { return spec()->channels; }

int Player::Audio::samples() const { return spec()->samples; }

[[maybe_unused]] void Player::Audio::setSampleRate(int sampleRate) const {
  spec()->sampleRate = sampleRate;
}

[[maybe_unused]] void
Player::Audio::setFormat(Player::Audio::AudioFormat format) {
  format_ = format;
}

[[maybe_unused]] void Player::Audio::setChannels(int channels) const {
  spec()->channels = channels;
}

[[maybe_unused]] void Player::Audio::setSamples(int samples) const {
  spec()->samples = samples;
}

int Player::Audio::bufferSize() { return samples() * bytesPerSample(); }

int Player::Audio::bytesPerSample() const {
  // SDL_AUDIO_BITSIZE(FormatF32LSB) -> 0x8120 & 0xff
  // return (SDL_AUDIO_BITSIZE(format()) * channels()) >> 3;
  return spec()->bitsPerSample * channels() >> 3;
}

Player::Spec *Player::Audio::spec() const { return spec_; }
