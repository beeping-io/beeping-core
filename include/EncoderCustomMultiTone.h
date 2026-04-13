
#ifndef __ENCODERCUSTOMMLTITONE__
#define __ENCODERCUSTOMMLTITONE__

#include <Encoder.h>

namespace BEEPING {
class EncoderCustomMultiTone : public Encoder {
 public:
  EncoderCustomMultiTone(const BeepingConfig& config, float samplingRate,
                         int buffsize, int windowSize);
  ~EncoderCustomMultiTone(void);

  float* mCurrentFreqs;
  float* mCurrentFreqsLoudness;

  int EncodeDataToAudioBuffer(const char* stringToEncode, int type, int size,
                              const char* melodyString, int melodySize);
  int GetEncodedAudioBuffer(float* audioBuffer);
  int ResetEncodedAudioBuffer();
};
}  // namespace BEEPING

#endif  //__ENCODERCUSTOMMLTITONE__