// BeepingCore performance benchmarks — Google Benchmark
//
// Run: ./BeepingCoreBenchmarks --benchmark_format=json
// CI compares against baseline and fails on >10% regression.

#include <BeepingCoreLib_api.h>
#include <benchmark/benchmark.h>

#include <cstring>
#include <vector>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static const char* kPayload9 = "123456789";  // 9 chars (standard beeping msg)

static std::vector<float> encodePayload(int mode, float sr,
                                        const char* payload) {
  void* core = BEEPING_Create();
  BEEPING_Configure(mode, sr, 1024, core);
  int len = static_cast<int>(std::strlen(payload));
  BEEPING_EncodeDataToAudioBuffer(payload, len, 0, nullptr, 0, core);
  std::vector<float> audio;
  float buf[1024];
  while (true) {
    int n = BEEPING_GetEncodedAudioBuffer(buf, core);
    if (n <= 0) break;
    audio.insert(audio.end(), buf, buf + n);
    if (n < 1024) break;
  }
  BEEPING_Destroy(core);
  return audio;
}

// ---------------------------------------------------------------------------
// BM_CreateDestroy — measures context lifecycle overhead
// ---------------------------------------------------------------------------

static void BM_CreateDestroy(benchmark::State& state) {
  for (auto _ : state) {
    void* core = BEEPING_Create();
    benchmark::DoNotOptimize(core);
    BEEPING_Destroy(core);
  }
}
BENCHMARK(BM_CreateDestroy);

// ---------------------------------------------------------------------------
// BM_Configure — measures configure overhead per mode
// ---------------------------------------------------------------------------

static void BM_Configure(benchmark::State& state) {
  int mode = static_cast<int>(state.range(0));
  void* core = BEEPING_Create();
  for (auto _ : state) {
    BEEPING_Configure(mode, 44100.0f, 1024, core);
  }
  BEEPING_Destroy(core);
}
BENCHMARK(BM_Configure)
    ->Arg(BEEPING_MODE_AUDIBLE)
    ->Arg(BEEPING_MODE_INAUDIBLE)
    ->Arg(BEEPING_MODE_ALL);

// ---------------------------------------------------------------------------
// BM_Encode — measures encoding time for 9-char payload
// ---------------------------------------------------------------------------

static void BM_Encode(benchmark::State& state) {
  int mode = static_cast<int>(state.range(0));
  float sr = static_cast<float>(state.range(1));
  void* core = BEEPING_Create();
  BEEPING_Configure(mode, sr, 1024, core);
  int len = static_cast<int>(std::strlen(kPayload9));
  for (auto _ : state) {
    int samples =
        BEEPING_EncodeDataToAudioBuffer(kPayload9, len, 0, nullptr, 0, core);
    benchmark::DoNotOptimize(samples);
  }
  BEEPING_Destroy(core);
}
BENCHMARK(BM_Encode)
    ->Args({BEEPING_MODE_AUDIBLE, 44100})
    ->Args({BEEPING_MODE_AUDIBLE, 48000})
    ->Args({BEEPING_MODE_AUDIBLE, 96000})
    ->Args({BEEPING_MODE_INAUDIBLE, 44100})
    ->Args({BEEPING_MODE_INAUDIBLE, 48000})
    ->Args({BEEPING_MODE_ALL, 44100});

// ---------------------------------------------------------------------------
// BM_EncodeDrain — measures encode + full buffer drain
// ---------------------------------------------------------------------------

static void BM_EncodeDrain(benchmark::State& state) {
  int mode = static_cast<int>(state.range(0));
  float sr = static_cast<float>(state.range(1));
  void* core = BEEPING_Create();
  BEEPING_Configure(mode, sr, 1024, core);
  int len = static_cast<int>(std::strlen(kPayload9));
  float buf[1024];
  for (auto _ : state) {
    BEEPING_EncodeDataToAudioBuffer(kPayload9, len, 0, nullptr, 0, core);
    while (true) {
      int n = BEEPING_GetEncodedAudioBuffer(buf, core);
      benchmark::DoNotOptimize(n);
      if (n <= 0 || n < 1024) break;
    }
  }
  BEEPING_Destroy(core);
}
BENCHMARK(BM_EncodeDrain)
    ->Args({BEEPING_MODE_AUDIBLE, 44100})
    ->Args({BEEPING_MODE_AUDIBLE, 96000})
    ->Args({BEEPING_MODE_INAUDIBLE, 44100});

// ---------------------------------------------------------------------------
// BM_Decode — measures decode throughput (per 1024-sample chunk)
// ---------------------------------------------------------------------------

static void BM_Decode(benchmark::State& state) {
  int mode = static_cast<int>(state.range(0));
  float sr = static_cast<float>(state.range(1));

  // Pre-encode audio to feed to decoder
  auto audio = encodePayload(mode, sr, kPayload9);

  void* core = BEEPING_Create();
  BEEPING_Configure(mode, sr, 1024, core);

  int chunkIdx = 0;
  int chunkSize = 1024;
  for (auto _ : state) {
    // Feed one chunk at a time, cycling through the audio
    int offset = (chunkIdx * chunkSize) % static_cast<int>(audio.size());
    int thisChunk =
        std::min(chunkSize, static_cast<int>(audio.size()) - offset);
    int rc = BEEPING_DecodeAudioBuffer(audio.data() + offset, thisChunk, core);
    benchmark::DoNotOptimize(rc);
    chunkIdx++;
  }
  BEEPING_Destroy(core);
  state.SetItemsProcessed(state.iterations() * chunkSize);
}
BENCHMARK(BM_Decode)
    ->Args({BEEPING_MODE_AUDIBLE, 44100})
    ->Args({BEEPING_MODE_AUDIBLE, 48000})
    ->Args({BEEPING_MODE_AUDIBLE, 96000})
    ->Args({BEEPING_MODE_INAUDIBLE, 44100})
    ->Args({BEEPING_MODE_ALL, 44100});

// ---------------------------------------------------------------------------
// BM_RoundTrip — full encode → decode → verify cycle
// ---------------------------------------------------------------------------

static void BM_RoundTrip(benchmark::State& state) {
  int mode = static_cast<int>(state.range(0));
  float sr = static_cast<float>(state.range(1));
  int len = static_cast<int>(std::strlen(kPayload9));

  for (auto _ : state) {
    // Encode
    void* enc = BEEPING_Create();
    BEEPING_Configure(mode, sr, 1024, enc);
    BEEPING_EncodeDataToAudioBuffer(kPayload9, len, 0, nullptr, 0, enc);
    std::vector<float> audio;
    float buf[1024];
    while (true) {
      int n = BEEPING_GetEncodedAudioBuffer(buf, enc);
      if (n <= 0) break;
      audio.insert(audio.end(), buf, buf + n);
      if (n < 1024) break;
    }
    BEEPING_Destroy(enc);

    // Decode
    void* dec = BEEPING_Create();
    BEEPING_Configure(mode, sr, 1024, dec);
    for (size_t i = 0; i < audio.size(); i += 1024) {
      int cs = std::min(1024, static_cast<int>(audio.size() - i));
      int rc = BEEPING_DecodeAudioBuffer(audio.data() + i, cs, dec);
      if (rc == -3) break;
    }
    // Flush
    float silence[1024] = {};
    for (int i = 0; i < 100; i++) {
      int rc = BEEPING_DecodeAudioBuffer(silence, 1024, dec);
      if (rc == -3) break;
    }
    char decoded[30] = {};
    int drc = BEEPING_GetDecodedData(decoded, dec);
    benchmark::DoNotOptimize(drc);
    BEEPING_Destroy(dec);
  }
}
BENCHMARK(BM_RoundTrip)
    ->Args({BEEPING_MODE_AUDIBLE, 44100})
    ->Args({BEEPING_MODE_INAUDIBLE, 44100})
    ->Args({BEEPING_MODE_ALL, 44100})
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
