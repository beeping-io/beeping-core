#include <BeepingConfig.h>
#include <BeepingDebug.h>

namespace BEEPING {

BeepingConfig compute_config(int windowSize, float /*sampleRate*/) {
  BeepingConfig cfg;

  // freq2Bin hardcoded at 44100 — separation of beeps in bins is constant
  // regardless of actual sample rate (decoder is mostly at 44.1kHz).
  // BEE-23 will make sample rate parametric and use this argument.
  float freq2Bin = (float)windowSize / 44100.f;

  cfg.nBinsOffsetForAudibleMultiTone = 12;
  cfg.freqOffsetForAudibleMultiTone =
      (float)cfg.nBinsOffsetForAudibleMultiTone / freq2Bin;

  cfg.nBinsOffsetForNonAudibleMultiTone = 4;
  cfg.freqOffsetForNonAudibleMultiTone =
      (float)cfg.nBinsOffsetForNonAudibleMultiTone / freq2Bin;

  BDEBUG("compute_config windowSize=%d -> freq2Bin=%.6f", windowSize, freq2Bin);
  BDEBUG("  audible: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForAudibleMultiTone, cfg.freqOffsetForAudibleMultiTone);
  BDEBUG("  nonAudible: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForNonAudibleMultiTone,
         cfg.freqOffsetForNonAudibleMultiTone);

  return cfg;
}

}  // namespace BEEPING
