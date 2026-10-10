// CodecGuardTest.cpp — BEE-2228 guard tests.
//
// (1) BEEPING_GetDecodedData must be safe to call before DECODE_COMPLETE
//     (was SIGSEGV in ReedSolomon::SetCode dereferencing an empty vector).
// (2) The codec wire format always emits 9 chars (5 data + 4 trailer);
//     this test pins the contract so a future refactor doesn't silently
//     break the scheduler payload split.

#include <BeepingCoreLib_api.h>

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
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

TEST_CASE("BEE-2228 guard: GetDecodedData survives all three decoder backends",
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
  INFO("decoded=" << r.decoded << " rc=" << r.rc << " out=\"" << r.out << "\"");
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
  INFO("decoded=" << r.decoded << " rc=" << r.rc << " out=\"" << r.out << "\"");
  REQUIRE(r.decoded == -3);
  REQUIRE(r.rc == 9);
  REQUIRE(r.out == "abc120000");
}

// -----------------------------------------------------------------------------
// BEE-2354 / Issue #76 guards:
// 1. ReedSolomon::SetMessage buffer overflow prevention (kk-1, kk, kk+1,
// kk+50).
// 2. BEEPING_EncodeDataToAudioBuffer public boundary validation (payload size &
// chars).
// 3. Sweep property test across 0..64 char payloads.
// -----------------------------------------------------------------------------

#include <Globals.h>
#include <ReedSolomon.h>

TEST_CASE(
    "BEE-2354: ReedSolomon::SetMessage clamps capacity and prevents heap "
    "overflow",
    "[bee-2354][reedsolomon]") {
  BEEPING::ReedSolomon rs;
  const int kk = rs.kk;  // 23
  REQUIRE(kk == 23);

  SECTION("Empty message zeroes internal buffer and returns true") {
    std::vector<int> emptyMsg;
    bool ok = rs.SetMessage(emptyMsg);
    REQUIRE(ok);
    REQUIRE(rs.msg_len == 0);
    for (int i = 0; i < kk; ++i) {
      REQUIRE(rs.data[i] == 0);
    }
    // Encode and GetCode must not crash
    rs.Encode();
    std::vector<int> code;
    rs.GetCode(code);
    REQUIRE(code.size() == static_cast<size_t>(rs.nn - rs.kk));
  }

  SECTION(
      "msg_len = kk - 1 (22 items): copies all, zero-pads last, returns true") {
    std::vector<int> msg(kk - 1);
    for (int i = 0; i < kk - 1; ++i) msg[i] = (i + 1) % 32;
    bool ok = rs.SetMessage(msg);
    REQUIRE(ok);
    REQUIRE(rs.msg_len == kk - 1);
    for (int i = 0; i < kk - 1; ++i) {
      REQUIRE(rs.data[i] == (i + 1) % 32);
    }
    REQUIRE(rs.data[kk - 1] == 0);  // zero-padded

    rs.Encode();
    std::vector<int> code;
    rs.GetCode(code);
    REQUIRE(code.size() == static_cast<size_t>(kk - 1 + (rs.nn - rs.kk)));
  }

  SECTION("msg_len = kk (23 items): exact capacity, returns true") {
    std::vector<int> msg(kk);
    for (int i = 0; i < kk; ++i) msg[i] = (i + 1) % 32;
    bool ok = rs.SetMessage(msg);
    REQUIRE(ok);
    REQUIRE(rs.msg_len == kk);
    for (int i = 0; i < kk; ++i) {
      REQUIRE(rs.data[i] == (i + 1) % 32);
    }

    rs.Encode();
    std::vector<int> code;
    rs.GetCode(code);
    REQUIRE(code.size() == static_cast<size_t>(rs.nn));
  }

  SECTION(
      "msg_len = kk + 1 (24 items): clamped to kk, returns false, NO "
      "overflow") {
    std::vector<int> msg(kk + 1);
    for (int i = 0; i < kk + 1; ++i) msg[i] = (i + 1) % 32;
    bool ok = rs.SetMessage(msg);
    REQUIRE_FALSE(ok);
    REQUIRE(rs.msg_len == kk);
    for (int i = 0; i < kk; ++i) {
      REQUIRE(rs.data[i] == (i + 1) % 32);
    }

    rs.Encode();
    std::vector<int> code;
    rs.GetCode(code);
    REQUIRE(code.size() == static_cast<size_t>(rs.nn));
  }

  SECTION(
      "msg_len = kk + 50 (73 items): clamped to kk, returns false, NO "
      "overflow") {
    std::vector<int> msg(kk + 50);
    for (int i = 0; i < kk + 50; ++i) msg[i] = (i + 1) % 32;
    bool ok = rs.SetMessage(msg);
    REQUIRE_FALSE(ok);
    REQUIRE(rs.msg_len == kk);
    for (int i = 0; i < kk; ++i) {
      REQUIRE(rs.data[i] == (i + 1) % 32);
    }

    rs.Encode();
    std::vector<int> code;
    rs.GetCode(code);
    REQUIRE(code.size() == static_cast<size_t>(rs.nn));
  }
}

TEST_CASE(
    "BEE-2354: BEEPING_EncodeDataToAudioBuffer public boundary input "
    "validation",
    "[bee-2354][api]") {
  void* enc = BEEPING_Create();
  BEEPING_Configure(BEEPING_MODE_AUDIBLE, 44100.0f, 1024, enc);

  SECTION("Rejects nullptr payload") {
    int rc = BEEPING_EncodeDataToAudioBuffer(nullptr, 5, 0, nullptr, 0, enc);
    REQUIRE(rc == -2);
  }

  SECTION("Rejects zero or negative size") {
    int rc1 = BEEPING_EncodeDataToAudioBuffer("abcde", 0, 0, nullptr, 0, enc);
    REQUIRE(rc1 == -2);
    int rc2 = BEEPING_EncodeDataToAudioBuffer("abcde", -5, 0, nullptr, 0, enc);
    REQUIRE(rc2 == -2);
  }

  SECTION("Rejects nullptr core handle") {
    int rc =
        BEEPING_EncodeDataToAudioBuffer("abcde", 5, 0, nullptr, 0, nullptr);
    REQUIRE(rc == -1);
  }

  SECTION("Rejects payload size exceeding 9 (including 10, 20, 21, 50)") {
    std::string s10 = "0123456789";
    REQUIRE(BEEPING_EncodeDataToAudioBuffer(s10.c_str(), 10, 0, nullptr, 0,
                                            enc) == -2);

    std::string s20 = "01234567890123456789";
    REQUIRE(BEEPING_EncodeDataToAudioBuffer(s20.c_str(), 20, 0, nullptr, 0,
                                            enc) == -2);

    std::string s21 = "012345678901234567890";
    REQUIRE(BEEPING_EncodeDataToAudioBuffer(s21.c_str(), 21, 0, nullptr, 0,
                                            enc) == -2);

    std::string s50(50, 'a');
    REQUIRE(BEEPING_EncodeDataToAudioBuffer(s50.c_str(), 50, 0, nullptr, 0,
                                            enc) == -2);
  }

  SECTION("Rejects non-base32 characters") {
    // Non-base32 chars: 'w', 'x', 'y', 'z', symbols
    REQUIRE(BEEPING_EncodeDataToAudioBuffer("abcdw", 5, 0, nullptr, 0, enc) ==
            -2);
    REQUIRE(BEEPING_EncodeDataToAudioBuffer("abcdx", 5, 0, nullptr, 0, enc) ==
            -2);
    REQUIRE(BEEPING_EncodeDataToAudioBuffer("abcdz", 5, 0, nullptr, 0, enc) ==
            -2);
    REQUIRE(BEEPING_EncodeDataToAudioBuffer("abc!1", 5, 0, nullptr, 0, enc) ==
            -2);
    REQUIRE(BEEPING_EncodeDataToAudioBuffer("abc 1", 5, 0, nullptr, 0, enc) ==
            -2);
  }

  SECTION("Accepts valid 5-char and 9-char payloads") {
    int rc5 = BEEPING_EncodeDataToAudioBuffer("abcde", 5, 0, nullptr, 0, enc);
    REQUIRE(rc5 > 0);

    int rc9 =
        BEEPING_EncodeDataToAudioBuffer("012345678", 9, 0, nullptr, 0, enc);
    REQUIRE(rc9 > 0);
  }

  BEEPING_Destroy(enc);
}

TEST_CASE(
    "BEE-2354: Property sweep across payload sizes 0..64 never corrupts memory",
    "[bee-2354][property]") {
  void* enc = BEEPING_Create();
  BEEPING_Configure(BEEPING_MODE_AUDIBLE, 44100.0f, 1024, enc);

  // Generate a valid base32 string of length 64
  const char* base32Chars = "0123456789abcdefghijklmnopqrstuv";
  std::string full64;
  for (int i = 0; i < 64; ++i) {
    full64.push_back(base32Chars[i % 32]);
  }

  for (int len = 0; len <= 64; ++len) {
    int rc = BEEPING_EncodeDataToAudioBuffer(full64.c_str(), len, 0, nullptr, 0,
                                             enc);
    if (len >= 1 && len <= 9) {
      REQUIRE(rc > 0);
    } else {
      REQUIRE(rc == -2);
    }
  }

  BEEPING_Destroy(enc);
}
