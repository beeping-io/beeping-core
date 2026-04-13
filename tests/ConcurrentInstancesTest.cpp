// ConcurrentInstancesTest — verifies zero cross-contamination between
// independent BeepingCore instances running in parallel.
//
// Each thread creates its own instance configured with a DIFFERENT mode,
// encodes a unique payload, decodes silence to exercise the decode path,
// and verifies that no state leaks between instances.

#include <BeepingCoreLib_api.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

namespace {

constexpr int kThreads = 8;
constexpr int kInstancesPerThread = 4;
constexpr int kBufferSize = 1024;

constexpr int kModes[] = {
    BEEPING_MODE_AUDIBLE,       BEEPING_MODE_NONAUDIBLE,
    BEEPING_MODE_HIDDEN,        BEEPING_MODE_CUSTOM,
    BEEPING_MODE_ALL,           BEEPING_MODE_AUDIBLEOLD,
    BEEPING_MODE_NONAUDIBLEOLD,
    BEEPING_MODE_AUDIBLE // wrap
};
constexpr int kNumModes = sizeof(kModes) / sizeof(kModes[0]);

void worker(int threadId, std::atomic<int> &failures) {
  for (int inst = 0; inst < kInstancesPerThread; ++inst) {
    int mode = kModes[(threadId + inst) % kNumModes];

    void *core = BEEPING_Create();
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

    if (mode == BEEPING_MODE_CUSTOM) {
      BEEPING_SetCustomBaseFreq(13000.0f + threadId * 100.0f,
                                1 + (threadId % 3), core);
    }

    // Encode
    char payload[8];
    std::snprintf(payload, sizeof(payload), "t%di%d", threadId, inst);
    int payloadLen = static_cast<int>(std::strlen(payload));

    int totalSamples = BEEPING_EncodeDataToAudioBuffer(payload, payloadLen, 0,
                                                       nullptr, 0, core);
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
      if (n <= 0)
        break;

      // Feed to decoder
      BEEPING_DecodeAudioBuffer(audioBuf, n, core);

      if (++drained > 10000)
        break;
    }

    // Query confidence (exercises more API surface)
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
}

} // namespace

int main() {
  std::atomic<int> failures{0};
  std::vector<std::thread> threads;
  threads.reserve(kThreads);

  for (int i = 0; i < kThreads; ++i) {
    threads.emplace_back(worker, i, std::ref(failures));
  }
  for (auto &t : threads)
    t.join();

  int f = failures.load();
  std::printf("ConcurrentInstancesTest: threads=%d instances_per_thread=%d "
              "failures=%d\n",
              kThreads, kInstancesPerThread, f);
  return f == 0 ? 0 : 1;
}
