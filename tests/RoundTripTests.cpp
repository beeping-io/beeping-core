// RoundTripTests.cpp — Encode-then-decode round-trip tests for beeping-core.
//
// These are the most critical tests: they verify the codec actually works
// end-to-end at multiple sample rates, all 3 modes, and every supported
// bufferSize. The bufferSize axis is the regression guard for BEE-2249, where
// any bufferSize > windowSize corrupted the payload (the decoder's frame ring
// buffer overflowed because it drained only up to the first event per call).

#include <BeepingCoreLib_api.h>

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <cstring>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Helper: full encode → decode round-trip at an arbitrary bufferSize.
// Returns the decoded payload (empty string on any failure).
// ---------------------------------------------------------------------------

static std::string roundTrip(int mode, float sampleRate, const char* payload,
                             int bufferSize) {
  // --- Encoder ---
  void* encoder = BEEPING_Create();
  if (!encoder) return "";

  if (BEEPING_Configure(mode, sampleRate, bufferSize, encoder) != 0) {
    BEEPING_Destroy(encoder);
    return "";
  }

  int payloadLen = static_cast<int>(std::strlen(payload));
  int totalSamples = BEEPING_EncodeDataToAudioBuffer(payload, payloadLen, 0,
                                                     nullptr, 0, encoder);
  if (totalSamples <= 0) {
    BEEPING_Destroy(encoder);
    return "";
  }

  // Drain all encoded audio. The drain buffer must match bufferSize:
  // GetEncodedAudioBuffer writes up to bufferSize samples per call.
  std::vector<float> audio;
  audio.reserve(static_cast<size_t>(totalSamples));
  std::vector<float> buf(static_cast<size_t>(bufferSize));
  while (true) {
    int n = BEEPING_GetEncodedAudioBuffer(buf.data(), encoder);
    if (n <= 0) break;
    audio.insert(audio.end(), buf.begin(), buf.begin() + n);
    if (n < bufferSize) break;
  }
  BEEPING_Destroy(encoder);

  // --- Decoder ---
  void* decoder = BEEPING_Create();
  if (!decoder) return "";

  if (BEEPING_Configure(mode, sampleRate, bufferSize, decoder) != 0) {
    BEEPING_Destroy(decoder);
    return "";
  }

  // Feed encoded audio to decoder in bufferSize-sized chunks.
  int decoded = -1;
  for (size_t i = 0; i < audio.size(); i += static_cast<size_t>(bufferSize)) {
    int chunkSize = std::min(bufferSize, static_cast<int>(audio.size() - i));
    decoded = BEEPING_DecodeAudioBuffer(audio.data() + i, chunkSize, decoder);
    if (decoded == -3) break;  // -3 = complete word decoded
  }

  // If not decoded yet, flush with silence to push remaining frames through.
  if (decoded != -3) {
    std::vector<float> silence(static_cast<size_t>(bufferSize), 0.0f);
    for (int flush = 0; flush < 400; ++flush) {
      decoded = BEEPING_DecodeAudioBuffer(silence.data(), bufferSize, decoder);
      if (decoded == -3) break;
    }
  }

  std::string out;
  if (decoded == -3) {
    char result[64] = {0};
    int rc = BEEPING_GetDecodedData(result, decoder);
    if (rc > 0) out.assign(result, static_cast<size_t>(rc));
  }

  BEEPING_Destroy(decoder);
  return out;
}

// The codec zero-pads the payload to numWordTokens (9) characters, so a 9-char
// payload round-trips to itself.
static constexpr const char* kPayload = "123456789";

// ===========================================================================
// Parametric round-trip matrix: sampleRate × mode × bufferSize.
//
// Every combination must recover the exact payload. bufferSize spans values
// below, equal to, and above the analysis window — the axis that exposed
// BEE-2249 (bufferSize > windowSize corrupted the payload).
// ===========================================================================

TEST_CASE("RoundTrip matrix (rate x mode x bufferSize)", "[roundtrip]") {
  float sampleRate =
      GENERATE(22050.0f, 24000.0f, 32000.0f, 44100.0f, 48000.0f, 96000.0f);

  auto mode =
      GENERATE(BEEPING_MODE_AUDIBLE, BEEPING_MODE_INAUDIBLE, BEEPING_MODE_ALL);

  int bufferSize = GENERATE(128, 256, 512, 1024, 2048, 4096, 8192);

  CAPTURE(sampleRate, mode, bufferSize);
  REQUIRE(roundTrip(mode, sampleRate, kPayload, bufferSize) == kPayload);
}
