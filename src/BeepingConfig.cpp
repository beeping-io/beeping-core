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

  cfg.nBinsOffsetForHiddenMultiTone = 3;
  cfg.freqOffsetForHiddenMultiTone =
      (float)cfg.nBinsOffsetForHiddenMultiTone / freq2Bin;

  cfg.nBinsOffsetForCustomMultiTone = 2 + cfg.beepsSeparationForCustomMultiTone;
  cfg.freqOffsetForCustomMultiTone =
      (float)cfg.nBinsOffsetForCustomMultiTone / freq2Bin;

  BDEBUG("compute_config windowSize=%d -> freq2Bin=%.6f", windowSize, freq2Bin);
  BDEBUG("  audible: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForAudibleMultiTone, cfg.freqOffsetForAudibleMultiTone);
  BDEBUG("  nonAudible: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForNonAudibleMultiTone,
         cfg.freqOffsetForNonAudibleMultiTone);
  BDEBUG("  hidden: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForHiddenMultiTone, cfg.freqOffsetForHiddenMultiTone);
  BDEBUG("  custom: nBins=%d freqOffset=%.4f",
         cfg.nBinsOffsetForCustomMultiTone, cfg.freqOffsetForCustomMultiTone);

  return cfg;
}

void recompute_custom_offsets(BeepingConfig& config, int windowSize) {
  float freq2Bin = (float)windowSize / 44100.f;
  config.nBinsOffsetForCustomMultiTone =
      2 + config.beepsSeparationForCustomMultiTone;
  config.freqOffsetForCustomMultiTone =
      (float)config.nBinsOffsetForCustomMultiTone / freq2Bin;
  BDEBUG("recompute_custom_offsets sep=%d -> nBins=%d freqOffset=%.4f",
         config.beepsSeparationForCustomMultiTone,
         config.nBinsOffsetForCustomMultiTone,
         config.freqOffsetForCustomMultiTone);
}

}  // namespace BEEPING
