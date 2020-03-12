#ifndef __DECODER__
#define __DECODER__

#include <vector>

#define MAX_DECODE_STRING_SIZE 30 //max decoded string size is 30

namespace BEEPING
{
  class SpectralAnalysis;
  class ReedSolomon;

  struct sTokenProbs {
    int idxToken;
    float energyRatioToken;
    int idxToneMax;
    int idxToneSecond;
    float energyRatioToneMax;
    float energyRatioToneSecond;
    float energyToken;
  };


  class Decoder
  {
  public:
    Decoder(float sr, int buffsize, int windowSize, int numTokens, int numTones);
    ~Decoder(void);

    virtual int DecodeAudioBuffer(float *audioBuffer, int size);
    virtual int GetDecodedData(char *stringDecoded);
    
    float GetConfidenceError();
    float GetConfidenceNoise();
    float GetConfidence();

    float GetReceivedBeepsVolume();

    int GetDecodedMode();

    virtual float GetDecodingBeginFreq();
    virtual float GetDecodingEndFreq();

    int GetSpectrum(float *spectrumBuffer);
    
    virtual int AnalyzeStartTokens(float *audioBuffer);
    virtual int AnalyzeToken(float *audioBuffer);

    virtual int ComputeStatsStartTokens(void);
    virtual int ComputeStats(void);

    int getSizeFilledFrameCircularBuffer();
    int getSizeFilledBlockCircularBuffer();
    int getSizeFilledBlockCircularBuffer(int mode);
       
    virtual int DeReverbToken(const int nbins, int *freqsBins);
    
    virtual float ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);
    virtual float ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame);

    float mSampleRate;
    int mBufferSize;

    int mDecoding;
    //For multiple decoding mode (will be set after first token found and guessed decoding mode)
    int mDecodingMode; //enum DECODING_MODE { NONAUDIBLE = 0, AUDIBLE = 1, HIDDEN = 2, CUSTOM = 3 };

    char mDecodedString[MAX_DECODE_STRING_SIZE]; //max decoded string size is 50

    SpectralAnalysis* mSpectralAnalysis;

    int mReadPosInFrameCircularBuffer; //For single decoding mode
    int *mReadPosInBlockCircularBufferArray; //For multiple decoding mode
    int mWritePosInFrameCircularBuffer;
    int mSizeFrameCircularBuffer;
    float *mCircularBufferFloat;
    float *mAnalBufferFloat;

    int mWindowSize;
    int mHopSize;

    int mNumTokens;
    int mNumTones;

    float mFreq2Bin;

    int *mFreqsBins; //For single decoding mode
    int **mFreqsBinsArray; //For multiple decoding mode

    int mBinWidth;
    int mSizeTokenBinAnal;
    float *mEvalTokenMags;
    float *mEvalToneMags;
    
    //Region of interest Bin Idxs
    int mBeginBin; //For single decoding mode
    int mEndBin; //For single decoding mode
    int *mBeginBinArray; //For multiple decoding mode
    int *mEndBinArray; //For multiple decoding mode

    //int mFirstTokenBinOffsetInBlock;
    //int mBeginBinBlock;
    //int mEndBinBlock;

    int idxFrontDoorToken1;
    int idxFrontDoorToken2;
    

    //Token Statistics
    float *mEnergy;
    float *mEnergyRatios;
    float *mEnergyStd;
    float *mEnergyDiff;

    float *mEnergyRatiosSorted;
    int *mEnergyRatiosIdx;

    //BlockStatistics (circular buffer dur = 2 * tokendur)
    int mReadPosInBlockCircularBuffer;
    int mWritePosInBlockCircularBuffer;
    int mSizeBlockCircularBuffer;

    int mnToleranceFrames; //10% tolerance
    
    float **mBlockSpecMag;

    //For Statistics (Confidence)
    sTokenProbs *mBlockTokenStatistics; //For single decoding mode
    sTokenProbs **mBlockTokenStatisticsArray; //For multiple decoding mode

    int *mBlockEnergyRatiosTokenIdx; //For single decoding mode
    int **mBlockEnergyRatiosTokenIdx1Array; //For multiple decoding mode
    int *mBlockEnergyStdTokenIdx;
    int *mBlockEnergyDiffTokenIdx;

    int *mBlockEnergyRatiosTokenIdx2; //For single decoding mode
    int **mBlockEnergyRatiosTokenIdx2Array; //For multiple decoding mode
    int *mBlockEnergyStdTokenIdx2;
    int *mBlockEnergyDiffTokenIdx2;

    int *mBlockEnergyRatiosTokenIdx3; //For single decoding mode
    int **mBlockEnergyRatiosTokenIdx3Array; //For multiple decoding mode

    int *mBlockEnergyRatiosTokenIdx4; //For single decoding mode
    int **mBlockEnergyRatiosTokenIdx4Array; //For multiple decoding mode

    int *mTokenRepetitions;
    //float *mLastBlockTokenEnergy;

    int mEndStartTokenPosInBlockCircularBuffer;
    double mAccumulatedDecodingFrames;
    
    int mMessageLength;
    ReedSolomon *mReedSolomon;
    std::vector<int> mDecodedValues;
    int *mDecodedValuesOrig; //For reed solomon statistics

    std::vector<float> mSumPerFrame;

    float mConfidenceEnergyRatios;
    float mConfidenceRepetitions;
    float mConfidenceCorrection;
    float mConfidence;

    float mReceivedBeepsVolume;

  };
}

#endif //__DECODER__
