#ifndef __DECODERAUDIBLE__
#define __DECODERAUDIBLE__

#include <vector>

#include "Decoder.h"

#define MAX_DECODE_STRING_SIZE 30 //max decoded string size is 30

namespace BEEPING
{
  class SpectralAnalysis;
  class ReedSolomon;
  //class Decoder;

  class DecoderAudible: public Decoder
  {
  public:
    DecoderAudible(float sr, int buffsize, int windowSize);
    ~DecoderAudible(void);

    int mSizeNighbBins;
    int mSizeNeighbTokenBinAnal;
    float *mEvalNeighbTokenMags;

    int DecodeAudioBuffer(float *audioBuffer, int size);
    int GetDecodedData(char *stringDecoded);

    int GetSpectrum(float *spectrumBuffer);
    
    int AnalyzeStartTokens(float *audioBuffer);
    int AnalyzeToken(float *audioBuffer);

    int ComputeStatsStartTokens(void);
    int ComputeStats(void);

    int getSizeFilledFrameCircularBuffer();
    int getSizeFilledBlockCircularBuffer();
       
    int DeReverbToken(const int nbins, int *freqsBins);
    
    float ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);
    float ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);
  };
}

#endif //__DECODERAUDIBLE__
