
#ifndef __ENCODERHIDDENMULTITONE__
#define __ENCODERHIDDENMULTITONE__

#include "Encoder.h"

namespace BEEPING
{
  class EncoderHiddenMultiTone : public Encoder
  {
  public:
    EncoderHiddenMultiTone(float samplingRate, int buffsize, int windowSize);
    ~EncoderHiddenMultiTone(void);

    float* mCurrentFreqs;
    float* mCurrentFreqsLoudness;

    int EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize);
    int GetEncodedAudioBuffer(float *audioBuffer);
    int ResetEncodedAudioBuffer();
  };
}

#endif //__ENCODERHIDDENMULTITONE__