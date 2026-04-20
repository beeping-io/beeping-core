#ifndef BEEPING_CLI_WAV_READER_H
#define BEEPING_CLI_WAV_READER_H

#include <cstdint>
#include <string>
#include <vector>

namespace beeping::cli {

struct WavData {
  std::vector<float> samples;  // mono, normalized to [-1.0, 1.0]
  float sample_rate = 0.0f;
  int channels = 0;
};

// Parses a canonical RIFF WAV file (PCM 16-bit). Returns empty error string
// on success, else a human-readable message. Mono or stereo accepted; stereo
// is downmixed by averaging channels. Samples are converted from int16 to
// float in [-1.0, 1.0].
std::string read_wav_file(const std::string& path, WavData& out);

}  // namespace beeping::cli

#endif  // BEEPING_CLI_WAV_READER_H
