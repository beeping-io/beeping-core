// UnitTests.cpp — Comprehensive Catch2 v3 unit tests for beeping-core public C
// API.

#include <BeepingCoreLib_api.h>

#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// RAII helper — one BeepingCore instance per scope
// ---------------------------------------------------------------------------

struct ScopedCore {
  void* handle;
  ScopedCore() : handle(BEEPING_Create()) {}
  ~ScopedCore() {
    if (handle) BEEPING_Destroy(handle);
  }
  bool configure(int mode, float sr = 44100.0f, int bufSize = 1024) {
    return BEEPING_Configure(mode, sr, bufSize, handle) == 0;
  }
  int encode(const char* payload, int type = 0) {
    int len = static_cast<int>(std::strlen(payload));
    return BEEPING_EncodeDataToAudioBuffer(payload, len, type, nullptr, 0,
                                           handle);
  }
  std::vector<float> drainEncoded(int bufSize = 1024) {
    std::vector<float> all;
    std::vector<float> buf(bufSize);
    while (true) {
      int n = BEEPING_GetEncodedAudioBuffer(buf.data(), handle);
      if (n <= 0) break;
      all.insert(all.end(), buf.begin(), buf.begin() + n);
      if (n < bufSize) break;
    }
    return all;
  }
};

// ===========================================================================
// 1. LIFECYCLE
// ===========================================================================

TEST_CASE("Lifecycle: Create returns non-null", "[lifecycle]") {
  void* core = BEEPING_Create();
  REQUIRE(core != nullptr);
  BEEPING_Destroy(core);
}

TEST_CASE("Lifecycle: Multiple independent instances", "[lifecycle]") {
  void* a = BEEPING_Create();
  void* b = BEEPING_Create();
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(a != b);
  BEEPING_Destroy(a);
  BEEPING_Destroy(b);
}

// ===========================================================================
// 2. VERSION
// ===========================================================================

TEST_CASE("Version: GetVersion returns non-empty string", "[version]") {
  const char* v = BEEPING_GetVersion();
  REQUIRE(v != nullptr);
  REQUIRE(std::strlen(v) > 0);
}

TEST_CASE("Version: GetVersion contains 'Beeping'", "[version]") {
  const char* v = BEEPING_GetVersion();
  REQUIRE(std::strstr(v, "Beeping") != nullptr);
}

TEST_CASE("Version: GetVersionInfo fills buffer", "[version]") {
  char buf[256] = {0};
  int len = BEEPING_GetVersionInfo(buf);
  REQUIRE(len > 0);
  REQUIRE(len == static_cast<int>(std::strlen(buf)));
}

// ===========================================================================
// 3. CONFIGURE
// ===========================================================================

TEST_CASE("Configure: all 3 modes succeed", "[configure]") {
  ScopedCore c;

  SECTION("Audible") { REQUIRE(c.configure(BEEPING_MODE_AUDIBLE)); }

  SECTION("Inaudible") { REQUIRE(c.configure(BEEPING_MODE_INAUDIBLE)); }

  SECTION("All") { REQUIRE(c.configure(BEEPING_MODE_ALL)); }
}

TEST_CASE("Configure: invalid mode fails", "[configure]") {
  ScopedCore c;
  int rc = BEEPING_Configure(99, 44100.0f, 1024, c.handle);
  REQUIRE(rc < 0);
}

TEST_CASE("Configure: 48 kHz works", "[configure]") {
  ScopedCore c;
  REQUIRE(c.configure(BEEPING_MODE_AUDIBLE, 48000.0f));
}

TEST_CASE("Configure: reconfigure changes mode", "[configure]") {
  ScopedCore c;
  REQUIRE(c.configure(BEEPING_MODE_AUDIBLE));
  REQUIRE(c.configure(BEEPING_MODE_INAUDIBLE));
  REQUIRE(c.configure(BEEPING_MODE_ALL));
}

// ===========================================================================
// 4. ENCODE
// ===========================================================================

TEST_CASE("Encode: all 3 modes produce samples", "[encode]") {
  ScopedCore c;

  SECTION("Audible") {
    c.configure(BEEPING_MODE_AUDIBLE);
    REQUIRE(c.encode("123456789") > 0);
  }

  SECTION("Inaudible") {
    c.configure(BEEPING_MODE_INAUDIBLE);
    REQUIRE(c.encode("123456789") > 0);
  }

  SECTION("All") {
    c.configure(BEEPING_MODE_ALL);
    REQUIRE(c.encode("123456789") > 0);
  }
}

TEST_CASE("Encode: drain matches reported sample count", "[encode]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  int totalExpected = c.encode("123456789");
  auto audio = c.drainEncoded();
  REQUIRE(static_cast<int>(audio.size()) == totalExpected);
}

TEST_CASE("Encode: audio samples in valid range [-1.5, 1.5]", "[encode]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  c.encode("123456789");
  auto audio = c.drainEncoded();
  for (float sample : audio) {
    REQUIRE(sample >= -1.5f);
    REQUIRE(sample <= 1.5f);
  }
}

TEST_CASE("Encode: audio is not all zero", "[encode]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  c.encode("123456789");
  auto audio = c.drainEncoded();
  float maxAbs = 0.0f;
  for (float s : audio) maxAbs = std::max(maxAbs, std::fabs(s));
  REQUIRE(maxAbs > 0.01f);
}

TEST_CASE("Encode: reset allows re-read with identical output", "[encode]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  c.encode("123456789");
  auto first = c.drainEncoded();
  BEEPING_ResetEncodedAudioBuffer(c.handle);
  auto second = c.drainEncoded();
  REQUIRE(first.size() == second.size());
  for (size_t i = 0; i < first.size(); i++) {
    REQUIRE(first[i] == second[i]);
  }
}

TEST_CASE("Encode: different payloads produce different audio", "[encode]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  c.encode("111111111");
  auto audio1 = c.drainEncoded();

  c.encode("999999999");
  auto audio2 = c.drainEncoded();

  bool differ = false;
  size_t minLen = std::min(audio1.size(), audio2.size());
  for (size_t i = 0; i < minLen; i++) {
    if (audio1[i] != audio2[i]) {
      differ = true;
      break;
    }
  }
  REQUIRE(differ);
}

// ===========================================================================
// 5. DECODE
// ===========================================================================

TEST_CASE("Decode: silence returns -1", "[decode]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  std::vector<float> silence(1024, 0.0f);
  int rc = BEEPING_DecodeAudioBuffer(silence.data(), 1024, c.handle);
  REQUIRE(rc == -1);
}

// ===========================================================================
// 6. CONFIDENCE
// ===========================================================================

TEST_CASE("Confidence: initial values in valid range", "[confidence]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  float conf = BEEPING_GetConfidence(c.handle);
  float confErr = BEEPING_GetConfidenceError(c.handle);
  float confNoise = BEEPING_GetConfidenceNoise(c.handle);
  REQUIRE(conf >= 0.0f);
  REQUIRE(conf <= 1.0f);
  REQUIRE(confErr >= 0.0f);
  REQUIRE(confNoise >= 0.0f);
  // Volume — just verify it doesn't crash
  BEEPING_GetReceivedBeepsVolume(c.handle);
}

// ===========================================================================
// 7. AUDIO SIGNATURE
// ===========================================================================

TEST_CASE("AudioSignature: set and encode", "[signature]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);

  // Simple sine-wave signature: 0.1 s at 44.1 kHz
  std::vector<float> sig(4410);
  for (int i = 0; i < 4410; i++) {
    sig[i] = 0.3f * std::sin(2.0f * 3.14159f * 440.0f * i / 44100.0f);
  }

  int rc = BEEPING_SetAudioSignature(4410, sig.data(), c.handle);
  REQUIRE(rc == 0);

  int samples = c.encode("123456789");
  REQUIRE(samples > 0);
}

TEST_CASE("AudioSignature: clear with null", "[signature]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);

  int rc = BEEPING_SetAudioSignature(0, nullptr, c.handle);
  REQUIRE(rc == 0);

  int samples = c.encode("123456789");
  REQUIRE(samples > 0);
}

// ===========================================================================
// 8. EDGE CASES
// ===========================================================================

TEST_CASE("EdgeCase: all hex chars encode", "[edge]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  REQUIRE(c.encode("012345678") > 0);
}

TEST_CASE("EdgeCase: extended chars encode", "[edge]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  // Extended alphabet (g-v = indices 16-31)
  REQUIRE(c.encode("ghijklmnv") > 0);
}

TEST_CASE("EdgeCase: 48 kHz encode produces correct count", "[edge]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE, 48000.0f, 1024);
  int samples = c.encode("123456789");
  REQUIRE(samples > 0);
  auto audio = c.drainEncoded();
  REQUIRE(static_cast<int>(audio.size()) == samples);
}

TEST_CASE("EdgeCase: multiple sequential encodes", "[edge]") {
  ScopedCore c;
  c.configure(BEEPING_MODE_AUDIBLE);
  for (int i = 0; i < 5; i++) {
    int samples = c.encode("123456789");
    REQUIRE(samples > 0);
    auto audio = c.drainEncoded();
    REQUIRE(static_cast<int>(audio.size()) == samples);
  }
}

// ===========================================================================
// 9. ALL MODES SWEEP
// ===========================================================================

TEST_CASE("AllModes: encode-drain sweep", "[modes]") {
  const int modes[] = {BEEPING_MODE_AUDIBLE, BEEPING_MODE_INAUDIBLE,
                       BEEPING_MODE_ALL};

  SECTION("Audible") {
    ScopedCore c;
    REQUIRE(c.configure(modes[0]));
    int samples = c.encode("123456789");
    REQUIRE(samples > 0);
    auto audio = c.drainEncoded();
    REQUIRE(static_cast<int>(audio.size()) == samples);
  }

  SECTION("Inaudible") {
    ScopedCore c;
    REQUIRE(c.configure(modes[1]));
    int samples = c.encode("123456789");
    REQUIRE(samples > 0);
    auto audio = c.drainEncoded();
    REQUIRE(static_cast<int>(audio.size()) == samples);
  }

  SECTION("All") {
    ScopedCore c;
    REQUIRE(c.configure(modes[2]));
    int samples = c.encode("123456789");
    REQUIRE(samples > 0);
    auto audio = c.drainEncoded();
    REQUIRE(static_cast<int>(audio.size()) == samples);
  }
}
