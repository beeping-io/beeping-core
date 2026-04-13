#include <BeepingConfig.h>
#include <BeepingDebug.h>

namespace BEEPING {

BeepingConfig compute_config(int windowSize, float sampleRate) {
  BeepingConfig cfg;

  // freq2Bin converts Hz to FFT bin index: bin = freq * freq2Bin.
  // Using the actual sample rate ensures correct frequency mapping
  // at any rate (44100, 48000, 96000, 32000, etc.)
  float freq2Bin = (float)windowSize / sampleRate;

  cfg.nBinsOffsetForAudibleMultiTone = 12;
  cfg.freqOffsetForAudibleMultiTone =
      (float)cfg.nBinsOffsetForAudibleMultiTone / freq2Bin;

  cfg.nBinsOffsetForNonAudibleMultiTone = 4;
  cfg.freqOffsetForNonAudibleMultiTone =
      (float)cfg.nBinsOffsetForNonAudibleMultiTone / freq2Bin;

  BDEBUG("compute_config windowSize=%d sampleRate=%.0f freq2Bin=%.6f",
         windowSize, sampleRate, freq2Bin);
  BDEBUG("  audible: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForAudibleMultiTone, cfg.freqOffsetForAudibleMultiTone);
  BDEBUG("  inaudible: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForNonAudibleMultiTone,
         cfg.freqOffsetForNonAudibleMultiTone);

  return cfg;
}

}  // namespace BEEPING
