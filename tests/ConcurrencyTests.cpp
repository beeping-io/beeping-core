// ConcurrencyTests.cpp — Thread-safety tests for beeping-core.
//
// Verifies that independent BeepingCore instances can be used concurrently
// from multiple threads without data races or cross-contamination.

#include <BeepingCoreLib_api.h>

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

namespace {

constexpr int kThreads = 8;
constexpr int kItersPerThread = 25;
constexpr int kInstancesPerThread = 4;
constexpr int kBufferSize = 1024;

constexpr int kModes[] = {
    BEEPING_MODE_AUDIBLE,
    BEEPING_MODE_INAUDIBLE,
    BEEPING_MODE_ALL,
};
constexpr int kNumModes = sizeof(kModes) / sizeof(kModes[0]);

}  // namespace

// ===========================================================================
// Multi-threaded encode: 8 threads x 25 iterations
// ===========================================================================

TEST_CASE("Concurrency: multi-threaded encode stress", "[concurrency]") {
  std::atomic<int> failures{0};
  std::vector<std::thread> threads;
  threads.reserve(kThreads);

  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([t, &failures]() {
      for (int iter = 0; iter < kItersPerThread; ++iter) {
        int mode = kModes[(t + iter) % kNumModes];

        void* core = BEEPING_Create();
        if (!core) {
          failures.fetch_add(1, std::memory_order_relaxed);
          return;
        }

        int rc = BEEPING_Configure(mode, 44100.0f, kBufferSize, core);
        if (rc < 0) {
          failures.fetch_add(1, std::memory_order_relaxed);
          BEEPING_Destroy(core);
          continue;
        }

        char payload[16];
        std::snprintf(payload, sizeof(payload), "%d%d", t, iter);
        int payloadLen = static_cast<int>(std::strlen(payload));

        int totalSamples = BEEPING_EncodeDataToAudioBuffer(payload, payloadLen,
                                                           0, nullptr, 0, core);
        if (totalSamples <= 0) {
          failures.fetch_add(1, std::memory_order_relaxed);
          BEEPING_Destroy(core);
          continue;
        }

        // Drain encoded audio
        float audioBuf[kBufferSize];
        int drained = 0;
        while (true) {
          int n = BEEPING_GetEncodedAudioBuffer(audioBuf, core);
          if (n <= 0 || n < kBufferSize) break;
          if (++drained > 10000) break;
        }

        BEEPING_Destroy(core);
      }
    });
  }

  for (auto& th : threads) th.join();
  REQUIRE(failures.load() == 0);
}

// ===========================================================================
// Concurrent instances: 8 threads x 4 instances, different modes per thread
// ===========================================================================

TEST_CASE("Concurrency: concurrent instances no cross-contamination",
          "[concurrency]") {
  std::atomic<int> failures{0};
  std::vector<std::thread> threads;
  threads.reserve(kThreads);

  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([t, &failures]() {
      for (int inst = 0; inst < kInstancesPerThread; ++inst) {
        int mode = kModes[(t + inst) % kNumModes];

        void* core = BEEPING_Create();
        if (!core) {
          failures.fetch_add(1, std::memory_order_relaxed);
          return;
        }

        int rc = BEEPING_Configure(mode, 44100.0f, kBufferSize, core);
        if (rc < 0) {
          failures.fetch_add(1, std::memory_order_relaxed);
          BEEPING_Destroy(core);
          continue;
        }

        // Encode a unique payload per thread+instance
        char payload[16];
        std::snprintf(payload, sizeof(payload), "t%di%d", t, inst);
        int payloadLen = static_cast<int>(std::strlen(payload));

        int totalSamples = BEEPING_EncodeDataToAudioBuffer(payload, payloadLen,
                                                           0, nullptr, 0, core);
        if (totalSamples <= 0) {
          failures.fetch_add(1, std::memory_order_relaxed);
          BEEPING_Destroy(core);
          continue;
        }

        // Drain encoded audio and feed to decoder (loopback)
        float audioBuf[kBufferSize];
        int drained = 0;
        while (true) {
          int n = BEEPING_GetEncodedAudioBuffer(audioBuf, core);
          if (n <= 0) break;

          // Feed to decoder path
          BEEPING_DecodeAudioBuffer(audioBuf, n, core);

          if (++drained > 10000) break;
        }

        // Exercise confidence / metrics queries
        BEEPING_GetConfidence(core);
        BEEPING_GetConfidenceError(core);
        BEEPING_GetConfidenceNoise(core);
        BEEPING_GetReceivedBeepsVolume(core);
        BEEPING_GetDecodedMode(core);

        if (mode >= BEEPING_MODE_AUDIBLE) {
          BEEPING_GetDecodingBeginFreq(core);
          BEEPING_GetDecodingEndFreq(core);
        }

        BEEPING_Destroy(core);
      }
    });
  }

  for (auto& th : threads) th.join();
  REQUIRE(failures.load() == 0);
}
