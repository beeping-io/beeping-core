// RoundTripTests.cpp — Encode-then-decode round-trip tests for beeping-core.
//
// These are the most critical tests: they verify the codec actually works
// end-to-end at multiple sample rates and all 3 modes.

#include <BeepingCoreLib_api.h>

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Helper: full encode → decode round-trip
// ---------------------------------------------------------------------------

static bool roundTrip(int mode, float sampleRate, const char* payload) {
  // --- Encoder ---
  void* encoder = BEEPING_Create();
  if (!encoder) return false;

  if (BEEPING_Configure(mode, sampleRate, 1024, encoder) != 0) {
    BEEPING_Destroy(encoder);
    return false;
  }

  int payloadLen = static_cast<int>(std::strlen(payload));
  int totalSamples = BEEPING_EncodeDataToAudioBuffer(payload, payloadLen, 0,
                                                     nullptr, 0, encoder);
  if (totalSamples <= 0) {
    BEEPING_Destroy(encoder);
    return false;
  }

  // Drain all encoded audio
  std::vector<float> audio;
  audio.reserve(totalSamples);
  std::vector<float> buf(1024);
  while (true) {
    int n = BEEPING_GetEncodedAudioBuffer(buf.data(), encoder);
    if (n <= 0) break;
    audio.insert(audio.end(), buf.begin(), buf.begin() + n);
    if (n < 1024) break;
  }
  BEEPING_Destroy(encoder);

  // --- Decoder ---
  void* decoder = BEEPING_Create();
  if (!decoder) return false;

  if (BEEPING_Configure(mode, sampleRate, 1024, decoder) != 0) {
    BEEPING_Destroy(decoder);
    return false;
  }

  // Feed encoded audio to decoder in chunks
  int decoded = -1;
  for (size_t i = 0; i < audio.size(); i += 1024) {
    int chunkSize = std::min(1024, static_cast<int>(audio.size() - i));
    decoded = BEEPING_DecodeAudioBuffer(audio.data() + i, chunkSize, decoder);
    if (decoded == -3) break;  // -3 = complete word decoded
  }

  // If not decoded yet, flush with silence to push remaining frames through
  if (decoded != -3) {
    std::vector<float> silence(1024, 0.0f);
    for (int flush = 0; flush < 200; ++flush) {
      decoded = BEEPING_DecodeAudioBuffer(silence.data(), 1024, decoder);
      if (decoded == -3) break;
    }
  }

  bool success = false;
  if (decoded == -3) {
    char result[64] = {0};
    int rc = BEEPING_GetDecodedData(result, decoder);
    if (rc > 0) {
      success = (std::string(result, rc) == std::string(payload));
    }
  }

  BEEPING_Destroy(decoder);
  return success;
}

// ===========================================================================
// Round-trip at 96000 Hz
// ===========================================================================

TEST_CASE("RoundTrip 96000 Hz", "[roundtrip][96000]") {
  SECTION("Audible") {
    REQUIRE(roundTrip(BEEPING_MODE_AUDIBLE, 96000.0f, "123456789"));
  }
  SECTION("Inaudible") {
    REQUIRE(roundTrip(BEEPING_MODE_INAUDIBLE, 96000.0f, "123456789"));
  }
  SECTION("All") {
    REQUIRE(roundTrip(BEEPING_MODE_ALL, 96000.0f, "123456789"));
  }
}

// ===========================================================================
// Round-trip at 48000 Hz
// ===========================================================================

TEST_CASE("RoundTrip 48000 Hz", "[roundtrip][48000]") {
  SECTION("Audible") {
    REQUIRE(roundTrip(BEEPING_MODE_AUDIBLE, 48000.0f, "123456789"));
  }
  SECTION("Inaudible") {
    REQUIRE(roundTrip(BEEPING_MODE_INAUDIBLE, 48000.0f, "123456789"));
  }
  SECTION("All") {
    REQUIRE(roundTrip(BEEPING_MODE_ALL, 48000.0f, "123456789"));
  }
}

// ===========================================================================
// Round-trip at 44100 Hz
// ===========================================================================

TEST_CASE("RoundTrip 44100 Hz", "[roundtrip][44100]") {
  SECTION("Audible") {
    REQUIRE(roundTrip(BEEPING_MODE_AUDIBLE, 44100.0f, "123456789"));
  }
  SECTION("Inaudible") {
    REQUIRE(roundTrip(BEEPING_MODE_INAUDIBLE, 44100.0f, "123456789"));
  }
  SECTION("All") {
    REQUIRE(roundTrip(BEEPING_MODE_ALL, 44100.0f, "123456789"));
  }
}

// ===========================================================================
// Round-trip at 32000 Hz
// ===========================================================================

TEST_CASE("RoundTrip 32000 Hz", "[roundtrip][32000]") {
  SECTION("Audible") {
    REQUIRE(roundTrip(BEEPING_MODE_AUDIBLE, 32000.0f, "123456789"));
  }
  SECTION("Inaudible") {
    REQUIRE(roundTrip(BEEPING_MODE_INAUDIBLE, 32000.0f, "123456789"));
  }
  SECTION("All") {
    REQUIRE(roundTrip(BEEPING_MODE_ALL, 32000.0f, "123456789"));
  }
}

// ===========================================================================
// Round-trip at 24000 Hz
// ===========================================================================

TEST_CASE("RoundTrip 24000 Hz", "[roundtrip][24000]") {
  SECTION("Audible") {
    REQUIRE(roundTrip(BEEPING_MODE_AUDIBLE, 24000.0f, "123456789"));
  }
  SECTION("Inaudible") {
    REQUIRE(roundTrip(BEEPING_MODE_INAUDIBLE, 24000.0f, "123456789"));
  }
  SECTION("All") {
    REQUIRE(roundTrip(BEEPING_MODE_ALL, 24000.0f, "123456789"));
  }
}

// ===========================================================================
// Round-trip at 22050 Hz
// ===========================================================================

TEST_CASE("RoundTrip 22050 Hz", "[roundtrip][22050]") {
  SECTION("Audible") {
    REQUIRE(roundTrip(BEEPING_MODE_AUDIBLE, 22050.0f, "123456789"));
  }
  SECTION("Inaudible") {
    REQUIRE(roundTrip(BEEPING_MODE_INAUDIBLE, 22050.0f, "123456789"));
  }
  SECTION("All") {
    REQUIRE(roundTrip(BEEPING_MODE_ALL, 22050.0f, "123456789"));
  }
}
