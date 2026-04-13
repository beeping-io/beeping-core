#ifndef __DECODERNONAUDIBLEMULTITONE__
#define __DECODERNONAUDIBLEMULTITONE__

#include <Decoder.h>

#include <vector>

#define MAX_DECODE_STRING_SIZE 30  // max decoded string size is 30

namespace BEEPING {
class SpectralAnalysis;
class ReedSolomon;
// class Decoder;

class DecoderNonAudibleMultiTone : public Decoder {
 public:
  DecoderNonAudibleMultiTone(const BeepingConfig& config, float sr,
                             int buffsize, int windowSize);
  ~DecoderNonAudibleMultiTone(void);

  int* mIdxs;

  int* mBlockEnergyRatiosMaxToneIdx;
  int* mBlockEnergyRatiosSecondToneIdx;
  int* mToneRepetitions;

  int* idxTonesFrontDoorToken1;
  int* idxTonesFrontDoorToken2;

  int DecodeAudioBuffer(float* audioBuffer, int size);
  int GetDecodedData(char* stringDecoded);

  int GetSpectrum(float* spectrumBuffer);

  int AnalyzeStartTokens(float* audioBuffer);
  int AnalyzeToken(float* audioBuffer);

  int ComputeStatsStartTokens(void);
  int ComputeStats(void);

  int getSizeFilledFrameCircularBuffer();
  int getSizeFilledBlockCircularBuffer();

  float ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width,
                                            int nbins,
                                            std::vector<float>& sumPerFrame);
  float ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins,
                                         std::vector<float>& sumPerFrame);
};
}  // namespace BEEPING

#endif  //__DECODERNONAUDIBLEMULTITONE__
