#include <BeepingConfig.h>
#include <BeepingDebug.h>

#include <algorithm>

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

  // Adaptive inaudible base frequency.
  // Default: 17800 Hz (original value, works at 44.1k+).
  // If Nyquist is too low, compress into the highest range that fits.
  //
  // The decoder accesses bins in range:
  //   [baseFreq_bin - binWidth,
  //    baseFreq_bin + tonesSpan_bins + 2*nBinsOffset + binWidth]
  // where tonesSpan_bins ≈ 3 * nBinsOffset * (numTones - 1).
  // Plus a small fudge to keep mBlockSpecMag[mSpecSize-1] accessible.
  float nyquist = sampleRate / 2.0f;
  float inaudibleSpan = cfg.freqOffsetForNonAudibleMultiTone * 3.0f *
                        (float)(cfg.numTonesNonAudibleMultiTone - 1);
  // Extra headroom for odd/even base-freq shifting (+2*offset in bins, Hz
  // equiv)
  float headroom = cfg.freqOffsetForNonAudibleMultiTone * 2.0f;
  float margin =
      400.0f;  // keep 400Hz below Nyquist (covers binWidth + rounding)

  float requiredSpan = inaudibleSpan + headroom + margin;

  if (17800.f + requiredSpan <= nyquist) {
    // Standard: plenty of room
    cfg.inaudibleBaseFreq = 17800.f;
  } else if (nyquist > requiredSpan + 1000.f) {
    // Compressed: push base down so tones fit under Nyquist
    cfg.inaudibleBaseFreq = nyquist - requiredSpan;
  } else {
    // Not enough room even with compression — set base anyway,
    // decoder will likely fail (physical limitation)
    cfg.inaudibleBaseFreq = std::max(1000.f, nyquist - requiredSpan);
  }

  BDEBUG("compute_config windowSize={} sampleRate={:.0f} freq2Bin={:.6f}",
         windowSize, sampleRate, freq2Bin);
  BDEBUG("  audible: nBins={} freqOffset={:.4f}",
         cfg.nBinsOffsetForAudibleMultiTone, cfg.freqOffsetForAudibleMultiTone);
  BDEBUG(
      "  inaudible: nBins={} freqOffset={:.4f} baseFreq={:.0f} nyquist={:.0f}",
      cfg.nBinsOffsetForNonAudibleMultiTone,
      cfg.freqOffsetForNonAudibleMultiTone, cfg.inaudibleBaseFreq, nyquist);

  return cfg;
}

}  // namespace BEEPING
