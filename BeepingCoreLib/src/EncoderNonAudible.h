#ifndef __ENCODERNONAUDIBLE__
#define __ENCODERNONAUDIBLE__

#include "Encoder.h"

namespace BEEPING
{
  class EncoderNonAudible: public Encoder
  {
  public:
    EncoderNonAudible(float samplingRate, int buffsize, int windowSize);
    ~EncoderNonAudible(void);

    int EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize);
    int GetEncodedAudioBuffer(float *audioBuffer);
    int ResetEncodedAudioBuffer();
  };
}

#endif //__ENCODERNONAUDIBLE__