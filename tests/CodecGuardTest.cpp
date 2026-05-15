// CodecGuardTest.cpp — BEE-2228 guard tests.
//
// (1) BEEPING_GetDecodedData must be safe to call before DECODE_COMPLETE
//     (was SIGSEGV in ReedSolomon::SetCode dereferencing an empty vector).
// (2) The codec wire format always emits 9 chars (5 data + 4 trailer);
//     this test pins the contract so a future refactor doesn't silently
//     break the scheduler payload split.

#include <BeepingCoreLib_api.h>

#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

namespace {

struct DecodeResult {
  int decoded = -1;
  int rc = 0;
  std::string out;
};

DecodeResult roundTrip(int mode, float sr, const char* payload, int bufSize) {
  void* enc = BEEPING_Create();
  BEEPING_Configure(mode, sr, bufSize, enc);
  int total = BEEPING_EncodeDataToAudioBuffer(
      payload, static_cast<int>(std::strlen(payload)), 0, nullptr, 0, enc);
  std::vector<float> audio;
  audio.reserve(total);
  std::vector<float> buf(bufSize);
  while (true) {
    int n = BEEPING_GetEncodedAudioBuffer(buf.data(), enc);
    if (n <= 0) break;
    audio.insert(audio.end(), buf.begin(), buf.begin() + n);
    if (n < bufSize) break;
  }
  BEEPING_Destroy(enc);

  void* dec = BEEPING_Create();
  BEEPING_Configure(mode, sr, bufSize, dec);
  int decoded = -1;
  for (size_t i = 0; i < audio.size(); i += bufSize) {
    int chunk = std::min(bufSize, static_cast<int>(audio.size() - i));
    decoded = BEEPING_DecodeAudioBuffer(audio.data() + i, chunk, dec);
    if (decoded == -3) break;
  }
  if (decoded != -3) {
    std::vector<float> silence(bufSize, 0.0f);
    for (int flush = 0; flush < 200; ++flush) {
      decoded = BEEPING_DecodeAudioBuffer(silence.data(), bufSize, dec);
      if (decoded == -3) break;
    }
  }
  DecodeResult r{decoded, 0, {}};
  if (decoded == -3) {
    char tmp[64] = {};
    r.rc = BEEPING_GetDecodedData(tmp, dec);
    r.out = std::string(tmp, static_cast<size_t>(std::abs(r.rc)));
  }
  BEEPING_Destroy(dec);
  return r;
}

}  // namespace

// ===========================================================================
// (1) Guard: GetDecodedData before DECODE_COMPLETE must not crash
// ===========================================================================

TEST_CASE(
    "BEE-2228 guard: GetDecodedData on a fresh decoder returns 0 and does "
    "not crash",
    "[bee-2228][guard]") {
  void* core = BEEPING_Create();
  REQUIRE(core != nullptr);
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 1024, core) == 0);
  char buf[64] = {};
  // Before the BEE-2228 fix this dereferenced ReedSolomon::recd over an
  // empty mDecodedValues vector — SIGSEGV.
  REQUIRE(BEEPING_GetDecodedData(buf, core) == 0);
  REQUIRE(buf[0] == '\0');
  BEEPING_Destroy(core);
}

TEST_CASE(
    "BEE-2228 guard: GetDecodedData survives all three decoder backends",
    "[bee-2228][guard]") {
  for (int mode :
       {BEEPING_MODE_AUDIBLE, BEEPING_MODE_INAUDIBLE, BEEPING_MODE_ALL}) {
    void* core = BEEPING_Create();
    REQUIRE(BEEPING_Configure(mode, 44100.0f, 1024, core) == 0);
    char buf[64] = {};
    INFO("mode=" << mode);
    REQUIRE(BEEPING_GetDecodedData(buf, core) == 0);
    BEEPING_Destroy(core);
  }
}

// ===========================================================================
// (2) Contract: decoded output is the 9-char payload-plus-trailer string
// ===========================================================================

TEST_CASE(
    "BEE-2228 contract: 9-char input round-trips exactly at the canonical "
    "bufSize=1024",
    "[bee-2228][contract]") {
  // This is the canonical happy-path: 9-char payload (5 data + 4 trailer)
  // round-trips byte-for-byte through encode→decode in-process. This
  // already worked before BEE-2228 — pin it so a future scheduler refactor
  // does not silently break it.
  auto r = roundTrip(BEEPING_MODE_INAUDIBLE, 44100.0f, "abcde0002", 1024);
  INFO("decoded=" << r.decoded << " rc=" << r.rc << " out=\"" << r.out
                  << "\"");
  REQUIRE(r.decoded == -3);
  REQUIRE(r.rc == 9);
  REQUIRE(r.out == "abcde0002");
}

TEST_CASE(
    "BEE-2228 contract: 5-char input is zero-padded to the 9-char wire "
    "format on decode",
    "[bee-2228][contract]") {
  // Documenting the protocol: passing a 5-char input to
  // BEEPING_EncodeDataToAudioBuffer encodes 5 data tokens, and the decoder
  // returns the same 5 chars followed by 4 trailing zeros. Callers that
  // want just the user payload must truncate to the first 5 chars.
  auto r = roundTrip(BEEPING_MODE_INAUDIBLE, 44100.0f, "abc12", 1024);
  INFO("decoded=" << r.decoded << " rc=" << r.rc << " out=\"" << r.out
                  << "\"");
  REQUIRE(r.decoded == -3);
  REQUIRE(r.rc == 9);
  REQUIRE(r.out == "abc120000");
}
