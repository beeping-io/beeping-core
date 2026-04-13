#ifndef __ENCODER__
#define __ENCODER__

#include <BeepingConfig.h>

namespace BEEPING {
class ReedSolomon;

class Encoder {
 public:
  Encoder(const BeepingConfig& config, float samplingRate, int buffsize,
          int windowSize, int numTokens, int numTones);
  virtual ~Encoder(void);

  int SetAudioSignature(int samplesSize, const float* samplesBuffer);

  ReedSolomon* mReedSolomon;
  virtual int EncodeDataToAudioBuffer(const char* stringToEncode, int type,
                                      int size, const char* melodyString,
                                      int melodySize);
  int GetEncodedAudioBuffer(float* audioBuffer);
  int ResetEncodedAudioBuffer();

  // float fastSin(float x);

  int mnAudioSignatureSamples;
  float* mAudioSignature;

  float* mAudioBufferEncodedString;

  BeepingConfig m_config;

  int mNumTokens;
  int mNumTones;

  float mSampleRate;
  int mReadIndexEncodedAudioBuffer;
  int mNumMaxSamplesEncodedString;
  int mNumSamplesEncodedString;
  int mBufferSize;

  int mWindowSize;
};
}  // namespace BEEPING

#endif  //__ENCODER__