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
  int numTonesAudibleMultiTone = 9;
  int numTonesNonAudibleMultiTone = 9;

  // Frequency offsets (computed by compute_config)
  int nBinsOffsetForAudibleMultiTone = 12;
  float freqOffsetForAudibleMultiTone = 258.398442f;
  int nBinsOffsetForNonAudibleMultiTone = 4;
  float freqOffsetForNonAudibleMultiTone = 86.1328141638f;

  // Adaptive inaudible base frequency (computed by compute_config)
  // At 44.1k+: 17800 Hz.  At lower rates: drops to fit under Nyquist.
  float inaudibleBaseFreq = 17800.f;

  // Synth parameters (used internally by encoders)
  float synthVolume = 0.f;

  // Front door tokens
  char frontDoorTokens[2] = {'1', 'o'};
};

// Pure function: computes derived config from FFT size and sample rate
BeepingConfig compute_config(int windowSize, float sampleRate);

}  // namespace BEEPING

#endif  // __BEEPINGCONFIG__
