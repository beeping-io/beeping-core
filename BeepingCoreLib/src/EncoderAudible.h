#ifndef __ENCODERAUDIBLE__
#define __ENCODERAUDIBLE__

#include "Encoder.h"

namespace BEEPING
{
  class EncoderAudible : public Encoder
  {
  public:
    EncoderAudible(float samplingRate, int buffsize, int windowSize);
    ~EncoderAudible(void);

    int EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize);
    int GetEncodedAudioBuffer(float *audioBuffer);
    int ResetEncodedAudioBuffer();
  };
}

#endif //__ENCODERAUDIBLE__