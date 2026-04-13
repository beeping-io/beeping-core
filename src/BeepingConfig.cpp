#include <BeepingConfig.h>

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

  return cfg;
}

void recompute_custom_offsets(BeepingConfig &config, int windowSize) {
  float freq2Bin = (float)windowSize / 44100.f;
  config.nBinsOffsetForCustomMultiTone =
      2 + config.beepsSeparationForCustomMultiTone;
  config.freqOffsetForCustomMultiTone =
      (float)config.nBinsOffsetForCustomMultiTone / freq2Bin;
}

} // namespace BEEPING
