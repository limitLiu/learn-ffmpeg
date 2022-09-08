#include "Utils/header.h"
#include "Utils/spec.h"
#include <fstream>
#include <iostream>
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
    av_log(nullptr, AV_LOG_ERROR, "%s\n", "Failed to call av_find_input_format.");
    return;
  }
  const char *device = DEVICE_NAME;
  int ret = avformat_open_input(&ctx_, device, fmt, nullptr);
  if (ret < 0) {
    log_error(ret);
    return;
  }
}

void Player::Recorder::deinit() { avformat_close_input(&ctx_); }

Player::Recorder::~Recorder() { deinit(); }

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
    av_packet_unref(&pkt);
  }
  file.flush();
  file.close();

  Spec spec(context());
  Header header(spec);
  fs::path p(filename());
  header.dataSize = file_size(p);
  pcm2Wav(header, filename(), p.replace_extension("wav").string());
  stop();
}

void Player::Recorder::stop() { recording_ = false; }

void Player::Recorder::setFilename(const std::string &filename) { filename_ = filename; }

std::string Player::Recorder::filename() const { return filename_; }

AVFormatContext *Player::Recorder::context() { return ctx_; }

void Player::Recorder::pcm2Wav(Header &header, const std::string &pcmFilename,
                               const std::string &wavFilename) {
  std::ifstream pcm(pcmFilename, std::ios::binary);
  if (!pcm.is_open()) {
    return;
  }
  header.chunkSize =
      header.dataSize + sizeof(Header) - sizeof(header.chunkID) - sizeof(header.chunkSize);

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

  wav.flush();
  wav.close();
  pcm.close();
}

void Player::Recorder::recordWAV() {
  if (recording_) {
    stop();
    return;
  }
  std::thread writeThread(&Player::Recorder::writeWAV, this);
  writeThread.detach();
}

void Player::Recorder::writeWAV() {
  if (filename().empty()) {
    return;
  }

  AVPacket pkt;
  std::ofstream file;
  file.open(filename(), std::ios::binary);
  if (!file.is_open()) {
    return;
  }
  Spec spec(context());
  Header header(spec);
  file.write(reinterpret_cast<const char *>(&header), sizeof(Header));
  recording_ = true;
  int ret;
  while (recording_) {
    ret = av_read_frame(context(), &pkt);
    if (ret == 0) {
      file.write(reinterpret_cast<const char *>(pkt.data), pkt.size);
      header.dataSize += pkt.size;
      auto ms = 1000.0 * header.dataSize / header.byteRate;
      std::cout << ms << std::endl;
    } else if (ret == AVERROR(EAGAIN)) {
      continue;
    } else {
      log_error(ret);
      break;
    }
    av_packet_unref(&pkt);
  }
  fs::path f(filename());
  file.seekp(sizeof(Header) - sizeof(header.dataSize));
  file.write(reinterpret_cast<const char *>(&header.dataSize), sizeof(header.dataSize));

  header.chunkSize = file_size(f) - sizeof(header.chunkID) - sizeof(header.chunkSize);
  file.seekp(sizeof(header.chunkID));
  file.write(reinterpret_cast<const char *>(&header.chunkSize), sizeof(header.chunkSize));

  file.flush();
  file.close();
  stop();
}

void Player::Recorder::resample(const std::string &inputName, int inputSampleRate,
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

void Player::Recorder::resample(Player::ResampleAudioSpec &input,
                                Player::ResampleAudioSpec &output) {
  resample(input.filename, input.sampleRate, input.fmt, input.channelLayout, output.filename,
           output.sampleRate, output.fmt, output.channelLayout);
}

void Player::Recorder::resample() const {
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

bool Player::Recorder::checkSampleFmt(const AVCodec *codec, AVSampleFormat fmt) {
  auto p = codec->sample_fmts;
  while (*p != AV_SAMPLE_FMT_NONE) {
    if (*p == fmt) {
      return true;
    }
    p++;
  }
  return false;
}

void Player::Recorder::pcm2AAC(Player::ResampleAudioSpec &spec, std::string aacFilename) {
  std::ifstream input;
  input.open(spec.filename, std::ios::binary);
  if (!input.is_open()) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", "Failed to open input file");
    return;
  }

  if (aacFilename.empty()) {
    fs::path p(spec.filename);
    aacFilename = p.replace_extension("aac").string();
  }

  std::ofstream output;
  output.open(aacFilename, std::ios::binary);
  if (!output.is_open()) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", "Failed to open output file");
    return;
  }

  AVCodecContext *ctx = nullptr;
  AVFrame *pcm = nullptr;
  AVPacket *pkt = nullptr;
  int ret;

  auto encoderName = "libfdk_aac";
  //  avcodec_find_encoder(AV_CODEC_ID_AAC);
  const AVCodec *codec = avcodec_find_encoder_by_name(encoderName);
  if (!codec) {
    av_log(nullptr, AV_LOG_ERROR, "Can not find encoder by %s\n", encoderName);
    return;
  }

  if (!checkSampleFmt(codec, spec.fmt)) {
    // need resample
    av_log(nullptr, AV_LOG_ERROR, "Unsupported sample format %s\n",
           av_get_sample_fmt_name(spec.fmt));
    return;
  }

  ctx = avcodec_alloc_context3(codec);
  if (!ctx) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", "Failed to call avcodec_alloc_context3");
    return;
  }

  ctx->sample_rate = spec.sampleRate;
  ctx->sample_fmt = spec.fmt;
  ctx->ch_layout = spec.channelLayout;
  /*
   ffmpeg -ar 44100 -ac 2 -f s16le -i ./resample.pcm -c:a libfdk_aac -profile:a aac_he_v2 -b:a 32k
   output.aac
   */
  //  ctx->bit_rate = 32000;
  //  ctx->profile = FF_PROFILE_AAC_HE_V2;
  AVDictionary *opts = nullptr;
  // ffmpeg -ar 44100 -ac 2 -f s16le -i ./resample.pcm -c:a libfdk_aac -vbr 5 output.aac
  av_dict_set(&opts, "vbr", "1", 0);
  ret = avcodec_open2(ctx, codec, &opts);
  if (ret < 0) {
    log_error(ret);
    goto end;
  }

  pcm = av_frame_alloc();
  if (!pcm) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", "Failed to call av_frame_alloc");
    goto end;
  }

  pcm->nb_samples = ctx->frame_size;
  pcm->format = ctx->sample_fmt;
  pcm->ch_layout = ctx->ch_layout;

  ret = av_frame_get_buffer(pcm, 0);
  if (ret < 0) {
    log_error(ret);
    goto end;
  }

  pkt = av_packet_alloc();
  if (!pkt) {
    av_log(nullptr, AV_LOG_ERROR, "%s\n", "Failed to call av_packet_alloc");
    goto end;
  }

  while (
      (ret = (int)input.read(reinterpret_cast<char *>(pcm->data[0]), pcm->linesize[0]).gcount()) >
      0) {
    if (ret < pcm->linesize[0]) {
      int bytes = av_get_bytes_per_sample((AVSampleFormat)pcm->format);
      int ch = pcm->ch_layout.nb_channels;
      pcm->nb_samples = ret / (bytes * ch);
    }
    if (encode(ctx, pcm, pkt, output) < 0) {
      goto end;
    }
  }
  encode(ctx, nullptr, pkt, output);

end:
  input.close();
  output.close();

  av_frame_free(&pcm);
  av_packet_free(&pkt);
  avcodec_free_context(&ctx);
}

void Player::Recorder::pcm2AAC() {
  ResampleAudioSpec input;
  input.filename = "../resources/resample.pcm";
  input.sampleRate = 44100;
  input.fmt = static_cast<AVSampleFormat>(AV_SAMPLE_FMT_S16);
  input.channelLayout = AV_CHANNEL_LAYOUT_STEREO;
  pcm2AAC(input);
}

int Player::Recorder::encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt,
                             std::ofstream &output) {
  int ret = avcodec_send_frame(ctx, frame);
  if (ret < 0) {
    log_error(ret);
    return ret;
  }
  while (true) {
    ret = avcodec_receive_packet(ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return 0;
    } else if (ret < 0) {
      log_error(ret);
      break;
    }
    output.write(reinterpret_cast<const char *>(pkt->data), pkt->size);
    av_packet_unref(pkt);
  }
  return ret;
}
