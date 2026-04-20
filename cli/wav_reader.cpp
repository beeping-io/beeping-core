#include "wav_reader.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace beeping::cli {

namespace {

struct Chunk {
  char id[4];
  uint32_t size;
};

bool read_exact(std::ifstream& f, void* buf, std::size_t n) {
  f.read(static_cast<char*>(buf), static_cast<std::streamsize>(n));
  return f.good();
}

bool id_equals(const char id[4], const char* expected) {
  return std::memcmp(id, expected, 4) == 0;
}

}  // namespace

std::string read_wav_file(const std::string& path, WavData& out) {
  std::ifstream f(path, std::ios::binary);
  if (!f) {
    return "cannot open file: " + path;
  }

  // RIFF header
  char riff[4];
  uint32_t file_size = 0;
  char wave[4];
  if (!read_exact(f, riff, 4) || !read_exact(f, &file_size, 4) ||
      !read_exact(f, wave, 4)) {
    return "truncated RIFF header";
  }
  if (!id_equals(riff, "RIFF") || !id_equals(wave, "WAVE")) {
    return "not a RIFF/WAVE file";
  }

  // Find fmt and data chunks
  uint16_t audio_format = 0;
  uint16_t channels = 0;
  uint32_t sample_rate = 0;
  uint16_t bits_per_sample = 0;
  std::vector<char> pcm_data;
  bool fmt_seen = false;
  bool data_seen = false;

  while (f && !(fmt_seen && data_seen)) {
    Chunk chunk{};
    if (!read_exact(f, chunk.id, 4) || !read_exact(f, &chunk.size, 4)) {
      break;
    }
    if (id_equals(chunk.id, "fmt ")) {
      if (chunk.size < 16) return "fmt chunk too small";
      uint16_t block_align = 0;
      uint32_t byte_rate = 0;
      if (!read_exact(f, &audio_format, 2) || !read_exact(f, &channels, 2) ||
          !read_exact(f, &sample_rate, 4) || !read_exact(f, &byte_rate, 4) ||
          !read_exact(f, &block_align, 2) ||
          !read_exact(f, &bits_per_sample, 2)) {
        return "truncated fmt chunk";
      }
      // Skip any extra bytes in the fmt chunk
      if (chunk.size > 16) {
        f.seekg(chunk.size - 16, std::ios::cur);
      }
      fmt_seen = true;
    } else if (id_equals(chunk.id, "data")) {
      pcm_data.resize(chunk.size);
      if (!read_exact(f, pcm_data.data(), chunk.size)) {
        return "truncated data chunk";
      }
      data_seen = true;
    } else {
      // Skip unknown chunk, pad byte if odd
      f.seekg(chunk.size + (chunk.size & 1u), std::ios::cur);
    }
  }

  if (!fmt_seen || !data_seen) {
    return "missing fmt or data chunk";
  }
  if (audio_format != 1 && audio_format != 3) {
    return "only PCM integer (1) or IEEE float (3) supported, got format " +
           std::to_string(audio_format);
  }
  if (channels < 1 || channels > 2) {
    return "only mono or stereo supported, got " + std::to_string(channels) +
           " channels";
  }
  if (audio_format == 1 && bits_per_sample != 16) {
    return "PCM integer only supports 16-bit, got " +
           std::to_string(bits_per_sample) + " bits";
  }
  if (audio_format == 3 && bits_per_sample != 32) {
    return "IEEE float only supports 32-bit, got " +
           std::to_string(bits_per_sample) + " bits";
  }

  const std::size_t bytes_per_sample = bits_per_sample / 8;
  const std::size_t frames = pcm_data.size() / (bytes_per_sample * channels);
  out.samples.resize(frames);

  if (audio_format == 1) {
    const auto* int16_data = reinterpret_cast<const int16_t*>(pcm_data.data());
    for (std::size_t i = 0; i < frames; ++i) {
      int32_t sum = 0;
      for (int c = 0; c < channels; ++c) {
        sum += int16_data[i * channels + c];
      }
      out.samples[i] = static_cast<float>(sum) / (channels * 32768.0f);
    }
  } else {
    // IEEE float 32-bit
    const auto* f32_data = reinterpret_cast<const float*>(pcm_data.data());
    for (std::size_t i = 0; i < frames; ++i) {
      float sum = 0.0f;
      for (int c = 0; c < channels; ++c) {
        sum += f32_data[i * channels + c];
      }
      out.samples[i] = sum / static_cast<float>(channels);
    }
  }
  out.sample_rate = static_cast<float>(sample_rate);
  out.channels = channels;
  return {};
}

}  // namespace beeping::cli
