
#ifndef __ENCODERNONAUDIBLEMULTITONE__
#define __ENCODERNONAUDIBLEMULTITONE__

#include "Encoder.h"

namespace BEEPING
{
  class EncoderNonAudibleMultiTone : public Encoder
  {
  public:
    EncoderNonAudibleMultiTone(float samplingRate, int buffsize, int windowSize);
    ~EncoderNonAudibleMultiTone(void);

    float* mCurrentFreqs;
    float* mCurrentFreqsLoudness;

    int EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize);
    int GetEncodedAudioBuffer(float *audioBuffer);
    int ResetEncodedAudioBuffer();
  };
}

#endif //__ENCODERNONAUDIBLEMULTITONE__