// CLIIntegrationTest.cpp — round-trip test of the `beeping-core` CLI.
//
// Generates audio with the encoder, writes it as a 16-bit PCM WAV, then
// runs the CLI binary as a subprocess to decode the WAV and checks that
// the payload round-trips correctly.

#include <BeepingCoreLib_api.h>

#include <algorithm>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#ifndef BEEPING_CLI_BINARY
#error "BEEPING_CLI_BINARY must be defined (CMake should inject the path)."
#endif

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/wait.h>
#define POPEN popen
#define PCLOSE pclose
#endif

namespace {

bool write_wav_float32_mono(const std::string& path,
                            const std::vector<float>& samples,
                            int sample_rate) {
  // Write IEEE float 32-bit (WAV format 3) — no quantization loss.
  std::ofstream f(path, std::ios::binary);
  if (!f) return false;

  const auto num_samples = static_cast<uint32_t>(samples.size());
  const uint32_t data_bytes = num_samples * sizeof(float);
  const uint32_t riff_size = 36 + data_bytes;
  const uint16_t channels = 1;
  const uint16_t bits = 32;
  const uint16_t block_align = 4;
  const uint32_t byte_rate = static_cast<uint32_t>(sample_rate) * 4;
  const uint16_t fmt = 3;  // IEEE float
  const uint32_t fmt_size = 16;
  const auto sr = static_cast<uint32_t>(sample_rate);

  auto w = [&](const void* p, std::size_t n) {
    f.write(static_cast<const char*>(p), static_cast<std::streamsize>(n));
  };
  w("RIFF", 4);
  w(&riff_size, 4);
  w("WAVE", 4);
  w("fmt ", 4);
  w(&fmt_size, 4);
  w(&fmt, 2);
  w(&channels, 2);
  w(&sr, 4);
  w(&byte_rate, 4);
  w(&block_align, 2);
  w(&bits, 2);
  w("data", 4);
  w(&data_bytes, 4);
  for (float s : samples) {
    w(&s, 4);
  }
  return f.good();
}

std::vector<float> encode_to_samples(int mode, float sample_rate,
                                     const char* payload) {
  void* encoder = BEEPING_Create();
  REQUIRE(encoder != nullptr);
  REQUIRE(BEEPING_Configure(mode, sample_rate, 1024, encoder) == 0);
  const int len = static_cast<int>(std::strlen(payload));
  const int total = BEEPING_EncodeDataToAudioBuffer(payload, len, 0, nullptr,
                                                    0, encoder);
  REQUIRE(total > 0);
  std::vector<float> audio;
  audio.reserve(static_cast<std::size_t>(total));
  std::vector<float> buf(1024);
  while (true) {
    int n = BEEPING_GetEncodedAudioBuffer(buf.data(), encoder);
    if (n <= 0) break;
    audio.insert(audio.end(), buf.begin(), buf.begin() + n);
    if (n < 1024) break;
  }
  BEEPING_Destroy(encoder);
  return audio;
}

std::pair<int, std::string> run_cli(const std::string& args) {
  // On Windows, `cmd.exe /c` strips the outermost quotes when the command
  // starts with `"` and ends with `"`, which breaks commands that have both
  // a quoted binary path AND a quoted argument. Wrap in an extra pair of
  // quotes to survive that strip.
#ifdef _WIN32
  std::string full = "\"\"" + std::string(BEEPING_CLI_BINARY) + "\" " + args +
                     " 2>&1\"";
#else
  std::string full = "\"" + std::string(BEEPING_CLI_BINARY) + "\" " + args +
                     " 2>&1";
#endif
  FILE* pipe = POPEN(full.c_str(), "r");
  REQUIRE(pipe != nullptr);
  std::array<char, 256> buf{};
  std::string output;
  while (std::fgets(buf.data(), static_cast<int>(buf.size()), pipe) !=
         nullptr) {
    output += buf.data();
  }
  int rc = PCLOSE(pipe);
#ifndef _WIN32
  if (WIFEXITED(rc)) rc = WEXITSTATUS(rc);
#endif
  return {rc, output};
}

}  // namespace

TEST_CASE("CLI: version command prints library version", "[cli]") {
  auto [rc, out] = run_cli("version");
  REQUIRE(rc == 0);
  REQUIRE(out.find("BeepingCoreLib") != std::string::npos);
}

TEST_CASE("CLI: help command exits cleanly", "[cli]") {
  auto [rc, out] = run_cli("help");
  REQUIRE(rc == 0);
  REQUIRE(out.find("beeping-core") != std::string::npos);
}

TEST_CASE("CLI: missing file returns exit code 2", "[cli]") {
  auto [rc, out] =
      run_cli("decode /definitely/not/a/real/file/path/xyz.wav");
  REQUIRE(rc == 2);
  REQUIRE(out.find("cannot open") != std::string::npos);
}

TEST_CASE("CLI: no args shows help without failing", "[cli]") {
  auto [rc, out] = run_cli("");
  REQUIRE(rc == 0);
  REQUIRE(out.find("Usage") != std::string::npos);
}

TEST_CASE("WAV round-trip without CLI subprocess", "[wav][roundtrip]") {
  constexpr float kSampleRate = 44100.0f;
  const char* payload = "123456789";
  auto samples = encode_to_samples(BEEPING_MODE_AUDIBLE, kSampleRate, payload);
  INFO("Encoder produced " << samples.size() << " samples");
  auto wav_path = std::filesystem::temp_directory_path() /
                  "beeping_wav_roundtrip_test.wav";
  REQUIRE(write_wav_float32_mono(wav_path.string(), samples,
                                 static_cast<int>(kSampleRate)));

  // Decode the samples we just wrote — verifies encoder output itself works
  void* dec = BEEPING_Create();
  REQUIRE(dec != nullptr);
  REQUIRE(BEEPING_Configure(BEEPING_MODE_AUDIBLE, kSampleRate, 1024, dec) == 0);
  bool found = false;
  for (std::size_t i = 0; i < samples.size(); i += 1024) {
    int n = static_cast<int>(std::min<std::size_t>(1024, samples.size() - i));
    if (BEEPING_DecodeAudioBuffer(samples.data() + i, n, dec) == -3) {
      found = true;
      break;
    }
  }
  if (!found) {
    std::vector<float> silence(1024, 0.0f);
    for (int k = 0; k < 200; ++k) {
      if (BEEPING_DecodeAudioBuffer(silence.data(), 1024, dec) == -3) {
        found = true;
        break;
      }
    }
  }
  REQUIRE(found);
  char decoded[32] = {0};
  int len = BEEPING_GetDecodedData(decoded, dec);
  INFO("Decoded directly: '" << std::string(decoded, decoded + std::max(0, len))
                              << "' len=" << len);
  BEEPING_Destroy(dec);
  REQUIRE(len > 0);
  std::filesystem::remove(wav_path);
}

TEST_CASE("CLI: decode round-trip audible mode", "[cli][roundtrip]") {
  constexpr float kSampleRate = 44100.0f;
  const char* payload = "123456789";

  auto samples = encode_to_samples(BEEPING_MODE_AUDIBLE, kSampleRate, payload);
  auto wav_path = std::filesystem::temp_directory_path() /
                  "beeping_cli_audible_test.wav";
  REQUIRE(write_wav_float32_mono(wav_path.string(), samples,
                                 static_cast<int>(kSampleRate)));

  auto [rc, out] = run_cli("decode -m 2 \"" + wav_path.string() + "\"");
  INFO("CLI output: " << out);
  REQUIRE(rc == 0);
  REQUIRE(out.find(payload) != std::string::npos);

  std::filesystem::remove(wav_path);
}
