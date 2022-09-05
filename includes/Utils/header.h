#ifndef PLAYER_HEADER_H
#define PLAYER_HEADER_H

#include "Core/recorder.h"

namespace Player {
struct Spec;

struct Header {
  [[maybe_unused]] const Uint8 chunkID[4] = {'R', 'I', 'F', 'F'};
  [[maybe_unused]] Uint32 chunkSize = 0;
  [[maybe_unused]] const Uint8 wave[4] = {'W', 'A', 'V', 'E'};
  [[maybe_unused]] const Uint8 subChunk1ID[4] = {'f', 'm', 't', ' '};
  [[maybe_unused]] const Uint32 subChunk1Size = 0x10;
  [[maybe_unused]] Uint16 audioFormat = 1;
  Uint16 numChannels = 0;
  Uint32 sampleRate = 0;
  [[maybe_unused]] Uint32 byteRate = 0;
  Uint16 blockAlign = 0;
  Uint16 bitsPerSample = 0;
  [[maybe_unused]] const Uint8 subChunk2ID[4] = {'d', 'a', 't', 'a'};
  Uint32 dataSize = 0;

  Header() = default;

  Header(Spec &spec);

  ~Header() = default;
};
} // namespace Player

#endif // PLAYER_HEADER_H
