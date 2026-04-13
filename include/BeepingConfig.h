#ifndef __BEEPINGCONFIG__
#define __BEEPINGCONFIG__

namespace BEEPING {
struct BeepingConfig {
  // Mathematical constants
  static constexpr float pi = 3.14159265358979323846f;
  static constexpr float two_pi = 2.f * 3.14159265358979323846f;

  // Timing parameters
  float durToken = 0.104489796f;
  float durFade = 0.075f;
  float tokenAmplitude = 0.7f;

  // Token/tone counts
  int numTokensAll = 32;
  int numTonesAll = 9;
  int numTokensAudible = 32;
  int numTokensNonAudible = 32;
  int numTokensHidden = 32;
  int numTokensCustom = 32;
  int numTonesAudibleMultiTone = 9;
  int numTonesNonAudibleMultiTone = 9;
  int numTonesHiddenMultiTone = 9;
  int numTonesCustomMultiTone = 9;

  // Frequency offsets (computed by compute_config)
  int nBinsOffsetForAudibleMultiTone = 12;
  float freqOffsetForAudibleMultiTone = 258.398442f;
  int nBinsOffsetForNonAudibleMultiTone = 4;
  float freqOffsetForNonAudibleMultiTone = 86.1328141638f;
  int nBinsOffsetForHiddenMultiTone = 3;
  float freqOffsetForHiddenMultiTone = 64.5996106228f;
  int nBinsOffsetForCustomMultiTone = 3;
  float freqOffsetForCustomMultiTone = 64.5996106228f;

  // Custom mode parameters (mutable per instance via API)
  float freqBaseForCustomMultiTone = 12000.f;
  int beepsSeparationForCustomMultiTone = 1;

  // Synth parameters (mutable per instance via API)
  int synthMode = 0;
  float synthVolume = 0.f;

  // Front door tokens
  char frontDoorTokens[2] = {'1', 'o'};
};

// Pure function: computes derived config from FFT size and sample rate
BeepingConfig compute_config(int windowSize, float sampleRate);

// Recompute custom mode offsets in-place after changing custom params
void recompute_custom_offsets(BeepingConfig &config, int windowSize);

} // namespace BEEPING

#endif // __BEEPINGCONFIG__
