// NoiseImmunityTest.cpp — Tests for decoder noise immunity, false-positive
// suppression, and clean packet decoding under noise. Part of BEE-2353 (#73).

#include <BeepingCoreLib_api.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

namespace {

// Deterministic LCG for reproducible synthetic noise across platforms
class SyntheticNoiseGenerator {
 public:
  explicit SyntheticNoiseGenerator(uint32_t seed = 42) : state_(seed) {}

  // Uniform white noise in [-amplitude, amplitude]
  float white(float amplitude = 0.1f) {
    state_ = state_ * 1664525u + 1013904223u;
    float norm =
        static_cast<float>(state_) / static_cast<float>(0xFFFFFFFFu);  // [0, 1]
    return (norm * 2.0f - 1.0f) * amplitude;
  }

  // Pink noise approximation using Voss-McCartney algorithm (3 octaves)
  float pink(float amplitude = 0.1f) {
    state_ = state_ * 1664525u + 1013904223u;
    int k = __builtin_ctz(state_ | (1u << 16)) % 4;
    pink_rows_[k] = white(amplitude * 0.5f);
    float sum = 0.0f;
    for (float r : pink_rows_) sum += r;
    return sum + white(amplitude * 0.25f);
  }

 private:
  uint32_t state_;
  float pink_rows_[4] = {0.0f, 0.0f, 0.0f, 0.0f};
};

}  // namespace

TEST_CASE(
    "NoiseImmunity: 60s silence never emits start or end tokens across modes",
    "[noise]") {
  auto mode =
      GENERATE(BEEPING_MODE_AUDIBLE, BEEPING_MODE_INAUDIBLE, BEEPING_MODE_ALL);
  CAPTURE(mode);

  void* decoder = BEEPING_Create();
  REQUIRE(decoder != nullptr);
  REQUIRE(BEEPING_Configure(mode, 44100.0f, 1024, decoder) == 0);

  const int bufferSize = 1024;
  std::vector<float> silence(bufferSize, 0.0f);

  // 60 seconds of pure digital silence (44100 * 60 / 1024 ≈ 2583 frames)
  const int totalFrames = (44100 * 60) / bufferSize;
  int startTokensSeen = 0;
  int endTokensSeen = 0;

  for (int f = 0; f < totalFrames; ++f) {
    int code = BEEPING_DecodeAudioBuffer(silence.data(), bufferSize, decoder);
    if (code == -2) startTokensSeen++;
    if (code == -3) endTokensSeen++;
  }

  CHECK(startTokensSeen == 0);
  CHECK(endTokensSeen == 0);

  char decodedData[64] = {0};
  int rc = BEEPING_GetDecodedData(decodedData, decoder);
  CHECK(rc <= 0);
  CHECK(decodedData[0] == '\0');

  BEEPING_Destroy(decoder);
}

TEST_CASE("NoiseImmunity: 60s pink noise never emits end tokens across modes",
          "[noise]") {
  auto mode =
      GENERATE(BEEPING_MODE_AUDIBLE, BEEPING_MODE_INAUDIBLE, BEEPING_MODE_ALL);
  CAPTURE(mode);

  void* decoder = BEEPING_Create();
  REQUIRE(decoder != nullptr);
  REQUIRE(BEEPING_Configure(mode, 44100.0f, 1024, decoder) == 0);

  SyntheticNoiseGenerator gen(67890 + mode);
  const int bufferSize = 1024;
  std::vector<float> noise(bufferSize);

  // 60 seconds of pink noise (1/f ambient room noise profile)
  const int totalFrames = (44100 * 60) / bufferSize;
  int endTokensSeen = 0;

  for (int f = 0; f < totalFrames; ++f) {
    for (int i = 0; i < bufferSize; ++i) {
      noise[i] = gen.pink(0.1f);
    }
    int code = BEEPING_DecodeAudioBuffer(noise.data(), bufferSize, decoder);
    if (code == -3) endTokensSeen++;
  }

  CHECK(endTokensSeen == 0);

  char decodedData[64] = {0};
  int rc = BEEPING_GetDecodedData(decodedData, decoder);
  CHECK(rc <= 0);
  CHECK(decodedData[0] == '\0');

  BEEPING_Destroy(decoder);
}

TEST_CASE("NoiseImmunity: 60s white noise never emits end tokens across modes",
          "[noise]") {
  auto mode =
      GENERATE(BEEPING_MODE_AUDIBLE, BEEPING_MODE_INAUDIBLE, BEEPING_MODE_ALL);
  CAPTURE(mode);

  void* decoder = BEEPING_Create();
  REQUIRE(decoder != nullptr);
  REQUIRE(BEEPING_Configure(mode, 44100.0f, 1024, decoder) == 0);

  SyntheticNoiseGenerator gen(12345 + mode);
  const int bufferSize = 1024;
  std::vector<float> noise(bufferSize);

  // 60 seconds of white noise at amplitude 0.05
  const int totalFrames = (44100 * 60) / bufferSize;
  int endTokensSeen = 0;

  for (int f = 0; f < totalFrames; ++f) {
    for (int i = 0; i < bufferSize; ++i) {
      noise[i] = gen.white(0.05f);
    }
    int code = BEEPING_DecodeAudioBuffer(noise.data(), bufferSize, decoder);
    if (code == -3) endTokensSeen++;
  }

  CHECK(endTokensSeen == 0);

  char decodedData[64] = {0};
  int rc = BEEPING_GetDecodedData(decodedData, decoder);
  CHECK(rc <= 0);
  CHECK(decodedData[0] == '\0');

  BEEPING_Destroy(decoder);
}
