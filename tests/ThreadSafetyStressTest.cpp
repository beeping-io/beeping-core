// ThreadSafetyStressTest — exercises BEEPING_* API from multiple threads.
//
// Purpose: surface data races in beeping-core's shared mutable state by
// running concurrent encode pipelines from N threads against TSan. The test
// is expected to FAIL (or at least report races) on the current legacy
// implementation — that failure IS the audit data referenced by ADR-007.
//
// After BEE-22 lands the instance-based refactor, this test must pass clean
// under TSan and the `continue-on-error: true` in tsan.yml will be removed.

#include <BeepingCoreLib_api.h>

#include <atomic>
#include <cstdio>
#include <cstring>
#include <thread>
#include <vector>

namespace {

constexpr int kThreads = 8;
constexpr int kItersPerThread = 25;
constexpr int kBufferSize = 1024;

void worker(int threadId, std::atomic<int>& failures)
{
  for (int iter = 0; iter < kItersPerThread; ++iter) {
    void* core = BEEPING_Create();
    if (core == nullptr) {
      failures.fetch_add(1, std::memory_order_relaxed);
      return;
    }

    int rc = BEEPING_Configure(BEEPING_MODE_AUDIBLE, 44100.0f, kBufferSize, core);
    if (rc < 0) {
      failures.fetch_add(1, std::memory_order_relaxed);
      BEEPING_Destroy(core);
      continue;
    }

    char payload[8];
    std::snprintf(payload, sizeof(payload), "%d%d", threadId, iter);
    int payloadLen = static_cast<int>(std::strlen(payload));

    BEEPING_EncodeDataToAudioBuffer(payload, payloadLen, 0, nullptr, 0, core);

    float audioBuf[kBufferSize];
    int drained = 0;
    while (true) {
      int n = BEEPING_GetEncodedAudioBuffer(audioBuf, core);
      if (n <= 0 || n < kBufferSize) break;
      if (++drained > 10000) break;
    }

    BEEPING_Destroy(core);
  }
}

} // namespace

int main(int argc, char** argv)
{
  bool singleThreaded = (argc > 1 && std::strcmp(argv[1], "--single") == 0);
  int nThreads = singleThreaded ? 1 : kThreads;

  std::atomic<int> failures{0};
  std::vector<std::thread> threads;
  threads.reserve(nThreads);

  for (int i = 0; i < nThreads; ++i) {
    threads.emplace_back(worker, i, std::ref(failures));
  }
  for (auto& t : threads) t.join();

  int f = failures.load();
  std::printf("ThreadSafetyStressTest: threads=%d iters=%d failures=%d\n",
              nThreads, kItersPerThread, f);
  return f == 0 ? 0 : 1;
}
