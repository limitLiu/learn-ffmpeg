#include "Core/audio.h"
#include "Core/recorder.h"
#include "Utils/header.h"
#include "Utils/spec.h"
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>

#define PCM_CODE 0x0001
#define MS_ADPCM_CODE 0x0002
#define IEEE_FLOAT_CODE 0x0003
#define ALAW_CODE 0x0006
#define MULAW_CODE 0x0007
#define IMA_ADPCM_CODE 0x0011

// (header[3] << 24) | (header[2] << 16) | (header[1] << 8) | header[0];
#define BIG_2_LITTLE(dstType, ...) (*(dstType *)((Byte[]){__VA_ARGS__}))

namespace fs = std::filesystem;

Player::Audio::Audio() { init(); }

Player::Audio::~Audio() { deletePtr(&spec_); }

[[maybe_unused]] Player::Audio::Audio(const std::string &name) {
  setFilename(name);
  init();
}

[[maybe_unused]] Player::Audio::Audio(const std::string &name, AVFormatContext *ctx) {
  setFilename(name);
  initWithFormatContext(ctx);
}

Player::Audio::Audio(AVFormatContext *ctx) { initWithFormatContext(ctx); }

void Player::Audio::init() { spec_ = new Spec(); }

void Player::Audio::initWithFormatContext(AVFormatContext *ctx) { spec_ = new Spec(ctx); }

std::string Player::Audio::filename() const { return filename_; }

void Player::Audio::setFilename(const std::string &name) { filename_ = name; }

void Player::Audio::play() {
  if (filename().empty()) {
    return;
  }

  if (playing_) {
    stop();
    return;
  }
  std::thread audio(&Player::Audio::run, this);
  audio.detach();
}

void Player::Audio::playWAV() {
  if (filename().empty()) {
    return;
  }
  fs::path p(filename());
  if (p.extension().string() != ".wav") {
    return;
  }
  std::thread audio(&Player::Audio::runWAV, this);
  audio.detach();
}

void Player::Audio::stop() { playing_ = false; }

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

void Player::Audio::runWAV() {
  SDL_AudioSpec spec;
  std::ifstream input;
  AudioBuffer audioBuffer;
  if (!(playing_ = parseWAV(spec, input))) {
    return;
  }
  spec.callback = pullAudioData;
  spec.userdata = &audioBuffer;

  if (SDL_OpenAudio(&spec, nullptr)) {
    return;
  }

  SDL_PauseAudio(0);

  auto bitsPerSample = SDL_AUDIO_BITSIZE(spec.format);
  auto bytesPerSample = (bitsPerSample * spec.channels) >> 3;
  auto bufSize = spec.samples * bytesPerSample;
  Byte buffer[bufSize];
  while (playing_) {
    if (audioBuffer.len) {
      continue;
    }
    audioBuffer.len = input.read(reinterpret_cast<char *>(buffer), bufferSize()).gcount();
    if (audioBuffer.len < 1) {
      auto ms = audioBuffer.pullSize / bytesPerSample / spec.freq;
      SDL_Delay(ms * 1000);
      break;
    }
    audioBuffer.data = buffer;
  }

  input.close();
  SDL_CloseAudio();
  playing_ = false;
}

bool Player::Audio::parseWAV(SDL_AudioSpec &spec, std::ifstream &input) const {
  bool success;
  std::vector<Byte> header;
  std::string chunkID;
  std::string format;
  std::string subChunk1ID;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint16_t bitsPerSample;

  input.open(filename(), std::ios::binary);
  if (!input.is_open()) {
    SDL_CloseAudio();
    success = false;
    goto end;
  }

  header.resize(0x2C);
  input.read(reinterpret_cast<char *>(&header[0]), 0x2C);

  // [0, 4)
  chunkID = std::string{&header[0], &header[4]};
  if (chunkID != "RIFF") {
    success = false;
    goto end;
  }

  // [8, 12)
  format = std::string{&header[8], &header[12]};
  if (format != "WAVE") {
    success = false;
    goto end;
  }

  subChunk1ID = std::string{&header[12], &header[16]};
  if (subChunk1ID != "fmt ") {
    success = false;
    goto end;
  }

  audioFormat = BIG_2_LITTLE(uint16_t, header[20], header[21]);
  numChannels = BIG_2_LITTLE(uint16_t, header[22], header[23]);
  spec.channels = numChannels;

  sampleRate = BIG_2_LITTLE(uint32_t, header[24], header[25], header[26], header[27]);
  spec.freq = (int)sampleRate;

  bitsPerSample = BIG_2_LITTLE(uint16_t, header[34], header[35]);

  spec.samples = samples();
  spec.format = getSDLFormat(audioFormat, bitsPerSample, success);

end:
  return success;
}

SDL_AudioFormat Player::Audio::getSDLFormat(uint16_t audioFormat, uint16_t bitsPerSample,
                                            bool &success) {
  SDL_AudioFormat format = 0;
  switch (audioFormat) {
  case MS_ADPCM_CODE:
  case IMA_ADPCM_CODE:
  case ALAW_CODE:
  case MULAW_CODE:
    format = AUDIO_S16SYS;
    success = true;
    break;
  case IEEE_FLOAT_CODE:
    format = AUDIO_F32LSB;
    success = true;
    break;
  case PCM_CODE:
    switch (bitsPerSample) {
    case 8:
      format = AUDIO_U8;
      success = true;
      break;
    case 16:
      format = AUDIO_S16LSB;
      success = true;
      break;
    case 24:
    case 32:
      format = AUDIO_S32LSB;
      success = true;
      break;
    default:
      success = false;
      break;
    }
    break;
  default:
    success = false;
    break;
  }
  return format;
}

void Player::Audio::run() {
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

  playing_ = true;
  Byte buffer[bufferSize()];
  while (playing_) {
    if (audioBuffer.len) {
      continue;
    }
    audioBuffer.len = input.read(reinterpret_cast<char *>(buffer), bufferSize()).gcount();
    if (audioBuffer.len < 1) {
      auto ms = audioBuffer.pullSize / bytesPerSample() / spec.freq;
      SDL_Delay(ms * 1000);
      break;
    }
    audioBuffer.data = buffer;
  }

  input.close();
  SDL_CloseAudio();
  playing_ = false;
}

int Player::Audio::sampleRate() const { return spec()->sampleRate; }

Player::Audio::AudioFormat Player::Audio::format() const { return format_; }

int Player::Audio::channels() const { return spec()->channels; }

int Player::Audio::samples() const { return spec()->samples; }

[[maybe_unused]] void Player::Audio::setSampleRate(int sampleRate) const {
  spec()->sampleRate = sampleRate;
}

[[maybe_unused]] void Player::Audio::setFormat(Player::Audio::AudioFormat format) {
  format_ = format;
}

[[maybe_unused]] void Player::Audio::setChannels(int channels) const {
  spec()->channels = channels;
}

[[maybe_unused]] void Player::Audio::setSamples(int samples) const { spec()->samples = samples; }

int Player::Audio::bufferSize() { return samples() * bytesPerSample(); }

int Player::Audio::bytesPerSample() const {
  // SDL_AUDIO_BITSIZE(FormatF32LSB) -> 0x8120 & 0xff
  // return (SDL_AUDIO_BITSIZE(format()) * channels()) >> 3;
  return spec()->bitsPerSample * channels() >> 3;
}

Player::Spec *Player::Audio::spec() const { return spec_; }

void Player::Audio::resample(const std::string &inputName, int inputSampleRate,
                             AVSampleFormat inputFmt, AVChannelLayout inputChLayout,
                             const std::string &outputName, int outputSampleRate,
                             AVSampleFormat outputFmt, AVChannelLayout outputChLayout) {
  std::ifstream input;
  std::ofstream output;

  input.open(inputName, std::ios::binary);
  if (!input.is_open()) {
    return;
  }

  output.open(outputName, std::ios::binary);
  if (!output.is_open()) {
    return;
  }

  Byte **inputData = nullptr;
  int inputLinesize = 0;
  int inputChannels = inputChLayout.nb_channels;
  int inputBytesPerSample = inputChannels * av_get_bytes_per_sample(inputFmt);
  int inputSamples = 1024;

  Byte **outputData = nullptr;
  int outputLinesize = 0;
  int outputChannels = outputChLayout.nb_channels;
  int outputSamples =
      (int)av_rescale_rnd(outputSampleRate, inputSamples, inputSampleRate, AV_ROUND_UP);
//  int outputBytesPerSample = outputChannels * av_get_bytes_per_sample(outputFmt);
  int len;
  SwrContext *ctx = nullptr;
  int ret = swr_alloc_set_opts2(&ctx, &outputChLayout, outputFmt, outputSampleRate, &inputChLayout,
                                inputFmt, inputSampleRate, 0, nullptr);
  if (ret < 0) {
    log_error(ret);
    goto end;
  }

  if ((ret = swr_init(ctx)) < 0) {
    log_error(ret);
    goto end;
  }

  ret = av_samples_alloc_array_and_samples(&inputData, &inputLinesize, inputChannels, inputSamples,
                                           inputFmt, 1);

  if (ret < 0) {
    log_error(ret);
    goto end;
  }

  ret = av_samples_alloc_array_and_samples(&outputData, &outputLinesize, outputChannels,
                                           outputSamples, outputFmt, 1);

  if (ret < 0) {
    log_error(ret);
    goto end;
  }

  while ((len = (int)input.read((char *)inputData[0], inputLinesize).gcount()) > 0) {
    inputSamples = len / inputBytesPerSample;
    ret = swr_convert(ctx, outputData, outputSamples, (const uint8_t **)inputData, inputSamples);
    if (ret < 0) {
      log_error(ret);
      goto end;
    }

    int size = av_samples_get_buffer_size(nullptr, outputChannels, ret, outputFmt, 1);
    output.write((char *)outputData[0], size);
  }

  while ((ret = swr_convert(ctx, outputData, outputSamples, nullptr, 0)) > 0) {
    int size = av_samples_get_buffer_size(nullptr, outputChannels, ret, outputFmt, 1);
    output.write((char *)outputData[0], size);
  }

end:
  input.close();
  output.close();

  if (inputData) {
    av_freep(&inputData[0]);
  }
  av_freep(&inputData);

  if (outputData) {
    av_freep(&outputData[0]);
  }
  av_freep(&outputData);

  swr_free(&ctx);
}

void Player::Audio::resample(Player::ResampleAudioSpec &input, Player::ResampleAudioSpec &output) {
  resample(input.filename, input.sampleRate, input.fmt, input.channelLayout, output.filename,
           output.sampleRate, output.fmt, output.channelLayout);
}

void Player::Audio::resample() const {
  ResampleAudioSpec input;
  ResampleAudioSpec output;

  input.filename = "../resources/out.pcm";
  input.fmt = static_cast<AVSampleFormat>(fmt_);
  input.sampleRate = 48000;
  input.channelLayout = AV_CHANNEL_LAYOUT_MONO;

  output.filename = "../resources/resample.pcm";
  output.fmt = static_cast<AVSampleFormat>(AV_SAMPLE_FMT_S16);
  output.sampleRate = 44100;
  output.channelLayout = AV_CHANNEL_LAYOUT_STEREO;

  resample(input, output);

  Spec spec;
  spec.channels = output.channelLayout.nb_channels;
  spec.sampleRate = output.sampleRate;
  spec.setCodecID(AV_CODEC_ID_PCM_S16LE);

  Header header(spec);
  fs::path p(output.filename);
  header.dataSize = file_size(p);
  p.replace_extension("wav");
  Player::Recorder::pcm2Wav(header, output.filename, p.string());
}
