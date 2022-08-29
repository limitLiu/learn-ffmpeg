#include "Utils/header.h"
#include "Utils/spec.h"

Player::Header::Header() = default;

Player::Header::~Header() = default;

Player::Header::Header(Player::Spec &spec) {
  numChannels = spec.channels;
  sampleRate = spec.sampleRate;
  bitsPerSample = spec.bitsPerSample;
  blockAlign = bitsPerSample * numChannels >> 3;
  byteRate = sampleRate * blockAlign;
  audioFormat = spec.codecID >= AV_CODEC_ID_PCM_F32BE ? 3 : 1;
}
