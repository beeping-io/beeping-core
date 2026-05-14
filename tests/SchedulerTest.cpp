// SchedulerTest.cpp — Catch2 v3 tests for the time-scheduled encoding C API
// (BEEPING_ComputeBeepSchedule, BEEPING_GetScheduleBufferSize,
// BEEPING_EncodeWithSchedule).

#include <BeepingCoreLib_api.h>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cmath>
#include <cstdint>
#include <vector>

using Catch::Matchers::WithinAbs;

// ===========================================================================
// BEEPING_ComputeBeepSchedule
// ===========================================================================

TEST_CASE("ComputeBeepSchedule: minimum duration yields a single beep",
          "[scheduler]") {
  double timestamps[16] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(2.3f, 0.0f, 2.3f, timestamps, 16,
                                           &count);
  REQUIRE(rc == 0);
  REQUIRE(count == 1);
  REQUIRE_THAT(timestamps[0], WithinAbs(0.0, 1e-6));
}

TEST_CASE("ComputeBeepSchedule: 10s @ 2.3s interval yields 4 beeps",
          "[scheduler]") {
  double timestamps[16] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(10.0f, 0.0f, 2.3f, timestamps, 16,
                                           &count);
  REQUIRE(rc == 0);
  REQUIRE(count == 4);
  REQUIRE_THAT(timestamps[0], WithinAbs(0.0, 1e-6));
  REQUIRE_THAT(timestamps[1], WithinAbs(2.3, 1e-3));
  REQUIRE_THAT(timestamps[2], WithinAbs(4.6, 1e-3));
  REQUIRE_THAT(timestamps[3], WithinAbs(6.9, 1e-3));
}

TEST_CASE("ComputeBeepSchedule: duration below kMinBeepWindow rejected",
          "[scheduler]") {
  double timestamps[4] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(2.0f, 0.0f, 2.3f, timestamps, 4,
                                           &count);
  REQUIRE(rc == -2);
}

TEST_CASE("ComputeBeepSchedule: startTime offset reduces count",
          "[scheduler]") {
  double timestamps[4] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(5.0f, 1.0f, 2.3f, timestamps, 4,
                                           &count);
  REQUIRE(rc == 0);
  REQUIRE(count == 1);
  REQUIRE_THAT(timestamps[0], WithinAbs(1.0, 1e-6));
}

TEST_CASE("ComputeBeepSchedule: negative interval rejected", "[scheduler]") {
  double timestamps[4] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(10.0f, 0.0f, -1.0f, timestamps, 4,
                                           &count);
  REQUIRE(rc == -2);
}

TEST_CASE(
    "ComputeBeepSchedule: negative startTime rejected",
    "[scheduler]") {
  double timestamps[4] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(10.0f, -0.1f, 2.3f, timestamps, 4,
                                           &count);
  REQUIRE(rc == -2);
}

TEST_CASE("ComputeBeepSchedule: startTime past duration rejected",
          "[scheduler]") {
  double timestamps[4] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(5.0f, 4.0f, 2.3f, timestamps, 4,
                                           &count);
  REQUIRE(rc == -2);
}

TEST_CASE(
    "ComputeBeepSchedule: size-query mode returns count without writing",
    "[scheduler]") {
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(10.0f, 0.0f, 2.3f, nullptr, 0,
                                           &count);
  REQUIRE(rc == 0);
  REQUIRE(count == 4);
}

TEST_CASE(
    "ComputeBeepSchedule: null buffer with maxTimestamps > 0 rejected",
    "[scheduler]") {
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(10.0f, 0.0f, 2.3f, nullptr, 4,
                                           &count);
  REQUIRE(rc == -1);
}

TEST_CASE(
    "ComputeBeepSchedule: truncation surfaced through outCount",
    "[scheduler]") {
  double timestamps[2] = {};
  int32_t count = -1;
  int32_t rc = BEEPING_ComputeBeepSchedule(10.0f, 0.0f, 2.3f, timestamps, 2,
                                           &count);
  REQUIRE(rc == 0);
  REQUIRE(count == 4);
  REQUIRE_THAT(timestamps[0], WithinAbs(0.0, 1e-6));
  REQUIRE_THAT(timestamps[1], WithinAbs(2.3, 1e-3));
}

TEST_CASE("ComputeBeepSchedule: closed-form parity across param sweeps",
          "[scheduler][property]") {
  struct Case {
    float duration;
    float startTime;
    float interval;
    int expected;
  };
  const Case cases[] = {
      {2.3f, 0.0f, 2.3f, 1},
      {4.6f, 0.0f, 2.3f, 2},
      {10.0f, 0.0f, 2.3f, 4},
      {30.0f, 0.0f, 5.0f, 6},
      {5.0f, 1.0f, 2.3f, 1},
      {5.0f, 0.5f, 2.3f, 1},
      {100.0f, 0.0f, 10.0f, 10},
      {7.0f, 2.0f, 2.3f, 2},
  };
  for (const auto& c : cases) {
    int32_t count = -1;
    int32_t rc = BEEPING_ComputeBeepSchedule(c.duration, c.startTime,
                                             c.interval, nullptr, 0, &count);
    INFO("d=" << c.duration << " s=" << c.startTime << " i=" << c.interval);
    REQUIRE(rc == 0);
    REQUIRE(count == c.expected);
  }
}

// ===========================================================================
// BEEPING_GetScheduleBufferSize
// ===========================================================================

TEST_CASE("GetScheduleBufferSize: returns duration * sampleRate",
          "[scheduler]") {
  void* core = BEEPING_Create();
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core) == 0);
  REQUIRE(BEEPING_GetScheduleBufferSize(10.0f, core) == 441000);
  REQUIRE(BEEPING_GetScheduleBufferSize(2.3f, core) ==
          static_cast<int32_t>(std::floor(2.3f * 44100.0f)));
  BEEPING_Destroy(core);
}

TEST_CASE("GetScheduleBufferSize: null handle returns -1", "[scheduler]") {
  REQUIRE(BEEPING_GetScheduleBufferSize(10.0f, nullptr) == -1);
}

TEST_CASE("GetScheduleBufferSize: zero duration returns -2", "[scheduler]") {
  void* core = BEEPING_Create();
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core) == 0);
  REQUIRE(BEEPING_GetScheduleBufferSize(0.0f, core) == -2);
  BEEPING_Destroy(core);
}

// ===========================================================================
// BEEPING_EncodeWithSchedule
// ===========================================================================

TEST_CASE("EncodeWithSchedule: 10s @ 44.1k produces 441000 non-silent samples",
          "[scheduler][encode]") {
  void* core = BEEPING_Create();
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core) == 0);

  const float duration = 10.0f;
  const int32_t required = BEEPING_GetScheduleBufferSize(duration, core);
  REQUIRE(required == 441000);

  std::vector<float> buf(static_cast<size_t>(required), 0.0f);
  int32_t written = -1;
  int32_t rc = BEEPING_EncodeWithSchedule("abcde", 5, 0, nullptr, 0, duration,
                                          0.0f, 2.3f, -3.0f, buf.data(),
                                          required, &written, core);
  REQUIRE(rc == 0);
  REQUIRE(written == required);

  bool hasNonZero = false;
  for (float s : buf) {
    if (std::fabs(s) > 1e-9f) {
      hasNonZero = true;
      break;
    }
  }
  REQUIRE(hasNonZero);

  BEEPING_Destroy(core);
}

TEST_CASE("EncodeWithSchedule: buffer too small returns required size",
          "[scheduler][encode]") {
  void* core = BEEPING_Create();
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core) == 0);

  std::vector<float> tooSmall(100, 0.0f);
  int32_t written = -1;
  int32_t rc = BEEPING_EncodeWithSchedule(
      "abcde", 5, 0, nullptr, 0, 10.0f, 0.0f, 2.3f, 0.0f, tooSmall.data(),
      static_cast<int32_t>(tooSmall.size()), &written, core);
  REQUIRE(rc == -1);
  REQUIRE(written == 441000);

  BEEPING_Destroy(core);
}

TEST_CASE("EncodeWithSchedule: unconfigured core returns -3",
          "[scheduler][encode]") {
  void* core = BEEPING_Create();
  std::vector<float> buf(1024, 0.0f);
  int32_t written = -1;
  int32_t rc = BEEPING_EncodeWithSchedule(
      "abcde", 5, 0, nullptr, 0, 10.0f, 0.0f, 2.3f, 0.0f, buf.data(),
      static_cast<int32_t>(buf.size()), &written, core);
  REQUIRE(rc == -3);
  BEEPING_Destroy(core);
}

TEST_CASE("EncodeWithSchedule: null code rejected", "[scheduler][encode]") {
  void* core = BEEPING_Create();
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core) == 0);
  std::vector<float> buf(441000, 0.0f);
  int32_t written = -1;
  int32_t rc = BEEPING_EncodeWithSchedule(
      nullptr, 0, 0, nullptr, 0, 10.0f, 0.0f, 2.3f, 0.0f, buf.data(),
      static_cast<int32_t>(buf.size()), &written, core);
  REQUIRE(rc == -2);
  BEEPING_Destroy(core);
}

// ===========================================================================
// BEEPING_ParseScheduledPayload
// ===========================================================================

TEST_CASE("ParseScheduledPayload: splits code + base32 timestamp",
          "[scheduler][parse]") {
  // Payload that EncodeWithSchedule emits for code="abcde", ts=2s (rounded).
  // toBase32(2) = "2", zero-padded to 4 chars => "0002".
  const char payload[] = "abcde0002";
  char codeBuf[16] = {};
  int32_t codeSize = -1;
  int32_t ts = -1;
  int32_t rc =
      BEEPING_ParseScheduledPayload(payload, 9, codeBuf, 16, &codeSize, &ts);
  REQUIRE(rc == 0);
  REQUIRE(codeSize == 5);
  REQUIRE(std::string(codeBuf) == "abcde");
  REQUIRE(ts == 2);
}

TEST_CASE("ParseScheduledPayload: timestamp via base32 [a-v]",
          "[scheduler][parse]") {
  // toBase32(10) = "a" (since alphabet is 0-9a-v), zero-padded => "000a".
  const char payload[] = "abcde000a";
  char codeBuf[16] = {};
  int32_t codeSize = -1;
  int32_t ts = -1;
  int32_t rc =
      BEEPING_ParseScheduledPayload(payload, 9, codeBuf, 16, &codeSize, &ts);
  REQUIRE(rc == 0);
  REQUIRE(ts == 10);
}

TEST_CASE("ParseScheduledPayload: large timestamp", "[scheduler][parse]") {
  // Pick a timestamp > 32 to exercise multi-digit base32. ts=2025 in base32:
  // 2025 / 32 = 63 r 9; 63 / 32 = 1 r 31; 1 / 32 = 0 r 1. So digits are
  // [1, 31, 9] = "1v9" -> 4-char padded => "01v9".
  const char payload[] = "abcde01v9";
  int32_t ts = -1;
  int32_t rc = BEEPING_ParseScheduledPayload(payload, 9, nullptr, 0, nullptr,
                                             &ts);
  REQUIRE(rc == 0);
  REQUIRE(ts == 2025);
}

TEST_CASE("ParseScheduledPayload: payload too short rejected",
          "[scheduler][parse]") {
  const char payload[] = "abcd";  // only 4 chars
  int32_t rc =
      BEEPING_ParseScheduledPayload(payload, 4, nullptr, 0, nullptr, nullptr);
  REQUIRE(rc == -2);
}

TEST_CASE("ParseScheduledPayload: invalid base32 in timestamp rejected",
          "[scheduler][parse]") {
  // 'z' is not in [0-9a-v].
  const char payload[] = "abcde00z2";
  int32_t rc =
      BEEPING_ParseScheduledPayload(payload, 9, nullptr, 0, nullptr, nullptr);
  REQUIRE(rc == -2);
}

TEST_CASE("ParseScheduledPayload: code buffer too small rejected",
          "[scheduler][parse]") {
  const char payload[] = "abcde0002";
  char codeBuf[4] = {};  // need 5 + 1 NUL = 6 minimum
  int32_t rc =
      BEEPING_ParseScheduledPayload(payload, 9, codeBuf, 4, nullptr, nullptr);
  REQUIRE(rc == -1);
}

TEST_CASE("ParseScheduledPayload: timestamp-only extraction without code",
          "[scheduler][parse]") {
  const char payload[] = "xyz123450005";  // 12 chars: code='xyz12345' ts=5
  int32_t codeSize = -1;
  int32_t ts = -1;
  int32_t rc = BEEPING_ParseScheduledPayload(payload, 12, nullptr, 0,
                                             &codeSize, &ts);
  REQUIRE(rc == 0);
  REQUIRE(codeSize == 8);
  REQUIRE(ts == 5);
}

TEST_CASE("ParseScheduledPayload: round-trip via toBase32 alphabet",
          "[scheduler][parse]") {
  // For every ts in {0, 1, 5, 31, 32, 100, 1023, 32767}, build a payload
  // emitted by EncodeWithSchedule (`code` + zero-padded base32) and verify
  // the parser recovers the original ts.
  const int candidates[] = {0, 1, 5, 31, 32, 100, 1023, 32767};
  for (int expected : candidates) {
    // Mimic the encoder's zero-padding logic.
    auto toB32 = [](int v) {
      static const char* alpha = "0123456789abcdefghijklmnopqrstuv";
      if (v <= 0) return std::string("0");
      std::string r;
      while (v > 0) {
        r.insert(r.begin(), alpha[v % 32]);
        v /= 32;
      }
      return r;
    };
    std::string b32 = toB32(expected);
    while (b32.size() < 4) b32.insert(b32.begin(), '0');
    std::string payload = std::string("abcde") + b32;

    int32_t ts = -1;
    int32_t rc = BEEPING_ParseScheduledPayload(
        payload.c_str(), static_cast<int32_t>(payload.size()), nullptr, 0,
        nullptr, &ts);
    INFO("expected ts=" << expected << " b32=" << b32);
    REQUIRE(rc == 0);
    REQUIRE(ts == expected);
  }
}

// ===========================================================================
// BEEPING_GetDecodedScheduledPayload
// ===========================================================================

TEST_CASE("GetDecodedScheduledPayload: returns 0 on unconfigured core",
          "[scheduler][parse]") {
  void* core = BEEPING_Create();
  char codeBuf[16] = {};
  int32_t rc = BEEPING_GetDecodedScheduledPayload(codeBuf, 16, nullptr, nullptr,
                                                  core);
  REQUIRE(rc == 0);
  BEEPING_Destroy(core);
}

TEST_CASE("GetDecodedScheduledPayload: returns 0 on null handle",
          "[scheduler][parse]") {
  char codeBuf[16] = {};
  int32_t rc =
      BEEPING_GetDecodedScheduledPayload(codeBuf, 16, nullptr, nullptr,
                                         nullptr);
  REQUIRE(rc == 0);
}

TEST_CASE("EncodeWithSchedule: beeps land at expected offsets",
          "[scheduler][encode]") {
  void* core = BEEPING_Create();
  REQUIRE(BEEPING_Configure(BEEPING_MODE_INAUDIBLE, 44100.0f, 128, core) == 0);

  const float duration = 10.0f;
  const float interval = 2.3f;
  const int32_t required = BEEPING_GetScheduleBufferSize(duration, core);
  std::vector<float> buf(static_cast<size_t>(required), 0.0f);
  int32_t written = -1;
  int32_t rc = BEEPING_EncodeWithSchedule("abcde", 5, 0, nullptr, 0, duration,
                                          0.0f, interval, -3.0f, buf.data(),
                                          required, &written, core);
  REQUIRE(rc == 0);

  // We expect 4 non-silent regions, one starting near each scheduled offset.
  const double offsets[] = {0.0, 2.3, 4.6, 6.9};
  for (double t : offsets) {
    const int32_t centerSample = static_cast<int32_t>(t * 44100.0) + 1000;
    REQUIRE(centerSample < required);
    bool foundEnergy = false;
    for (int32_t i = std::max(0, centerSample - 200);
         i < std::min(required, centerSample + 200); ++i) {
      if (std::fabs(buf[static_cast<size_t>(i)]) > 1e-6f) {
        foundEnergy = true;
        break;
      }
    }
    INFO("scheduled offset t=" << t << "s (sample " << centerSample << ")");
    REQUIRE(foundEnergy);
  }

  BEEPING_Destroy(core);
}
