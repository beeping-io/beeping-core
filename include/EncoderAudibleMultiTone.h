
#ifndef __ENCODERAUDIBLEMULTITONE__
#define __ENCODERAUDIBLEMULTITONE__

#include <Encoder.h>

namespace BEEPING {
class EncoderAudibleMultiTone : public Encoder {
 public:
  EncoderAudibleMultiTone(const BeepingConfig& config, float samplingRate,
                          int buffsize, int windowSize);
  ~EncoderAudibleMultiTone(void);

  float* mCurrentFreqs;
  float* mCurrentFreqsLoudness;

  int EncodeDataToAudioBuffer(const char* stringToEncode, int type, int size,
                              const char* melodyString, int melodySize);
  int GetEncodedAudioBuffer(float* audioBuffer);
  int ResetEncodedAudioBuffer();
};
}  // namespace BEEPING

#endif  //__ENCODERAUDIBLEMULTITONE__