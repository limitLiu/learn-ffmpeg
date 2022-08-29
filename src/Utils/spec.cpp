#include "Utils/spec.h"

Player::Spec::Spec(AVFormatContext *ctx) {
  if (!ctx) {
    return;
  }
  auto stream = ctx->streams[0];
  if (stream) {
    auto params = stream->codecpar;
    channels = params->channels;
    sampleRate = params->sample_rate;
    samples = 1024;
    //      bitPerSample = (params->channels *
    //                      av_get_bytes_per_sample((AVSampleFormat)params->format))
    //                     << 3;
    codecID = params->codec_id;
    bitsPerSample = av_get_bits_per_sample(codecID);
  }
}
