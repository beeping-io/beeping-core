#ifndef __DECODERALLMULTITONE__
#define __DECODERALLMULTITONE__

#include <vector>

#include "Decoder.h"

#define MAX_DECODE_STRING_SIZE 30 //max decoded string size is 30

namespace BEEPING
{
  class SpectralAnalysis;
  class ReedSolomon;
  //class Decoder;

  class DecoderAllMultiTone: public Decoder
  {
  public:
    DecoderAllMultiTone(float sr, int buffsize, int windowSize);
    ~DecoderAllMultiTone(void);

    int *mIdxs;

    int *mBlockEnergyRatiosMaxToneIdx;
    int *mBlockEnergyRatiosSecondToneIdx;
    int *mToneRepetitions;

    int *idxTonesFrontDoorToken1; //For single decoding mode
    int *idxTonesFrontDoorToken2; //For single decoding mode
    int **idxTonesFrontDoorToken1Array; //For multiple decoding mode
    int **idxTonesFrontDoorToken2Array; //For multiple decoding mode

    int DecodeAudioBuffer(float *audioBuffer, int size);
    int GetDecodedData(char *stringDecoded);

    int GetSpectrum(float *spectrumBuffer);
    
    int AnalyzeStartTokens(float *audioBuffer);
    int AnalyzeToken(float *audioBuffer, int mode);

    int ComputeStatsStartTokens(int mode);
    int ComputeStats(int mode);

    int getSizeFilledFrameCircularBuffer();
    int getSizeFilledBlockCircularBuffer();
    int getSizeFilledBlockCircularBuffer(int mode);
    
    float ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);
    float ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);
  };
}

#endif //__DECODERALLMULTITONE__
