#include "Decoder.h"
#include "ReedSolomon.h"

#include <cstdio>
#include <string.h>
#include <math.h>
#include <vector>
#include <numeric> //accumulate

#include "Globals.h"

#include "SpectralAnalysis.h"

#ifdef DEBUG_OUTPUT
#include <iostream>
#endif //DEBUG_OUTPUT


using namespace BEEPING;

Decoder::Decoder(float samplingRate, int buffSize, int windowSize, int numTokens, int numTones)
{
  mNumTokens = numTokens;
  mNumTones = numTones;

  mDecoding = 0;
  mDecodingMode = -1; //Mode between enum DECODING_MODE { NONAUDIBLE = 0, AUDIBLE = 1, HIDDEN = 2, CUSTOM = 3 };
  mDecodedString[0] = '\0';

  mSampleRate = samplingRate;
  mBufferSize = buffSize;
  mWindowSize = windowSize;
  	
  //decide hopsize and windowSize depending on sampleRate
  
  if (mSampleRate == 44100.0)
  {
#if (TARGET_OS_IPHONE)
    mHopSize = 256; //high cpu //was 128
#elif (TARGET_OS_SIMULATOR)
    mHopSize = 256;
#else
    mHopSize = 256; //low cpu
#endif
    //mHopSize = 512; 
  }
  else if (mSampleRate == 22050.0)
  {
#ifdef TARGET_OS_IPHONE
    mHopSize = 128; //high cpu //was 64
#else
    mHopSize = 128; //low cpu
#endif
    //mHopSize = 256;
  }
  else if (mSampleRate == 11050.0)
  {
#ifdef TARGET_OS_IPHONE
    mHopSize = 64; //high cpu //was 32
#else
    mHopSize = 64; //low cpu
#endif
    //mHopSize = 128;
  }
  else //not tested
  {
    //invalid samplerate
    mHopSize = 64;
  }

  Globals::init(windowSize, mSampleRate);

  //fftSize = windowSize
  mSpectralAnalysis = new SpectralAnalysis(kEnergySpectrum, mWindowSize, mWindowSize, mHopSize); //fftsize = 2048, windowsize=2048, hopsize = 512

  mReadPosInFrameCircularBuffer = 0;
  mWritePosInFrameCircularBuffer = 0;
  mSizeFrameCircularBuffer = mSpectralAnalysis->mWindowSize * 4;
  mCircularBufferFloat = new float[mSizeFrameCircularBuffer];
  memset(mCircularBufferFloat,0,mSizeFrameCircularBuffer*sizeof(float));
  mAnalBufferFloat = new float[mSpectralAnalysis->mWindowSize];
  memset(mAnalBufferFloat,0,mSpectralAnalysis->mWindowSize*sizeof(float));

  mBinWidth = 1; //is 2, maybe should be changed to 1 for closer freqs (non audible multitone)
  //mBinWidth = 1;

  mSizeTokenBinAnal = mBinWidth * 2 + 1;
  mEvalTokenMags = new float[mSizeTokenBinAnal];
  memset(mEvalTokenMags,0,mSizeTokenBinAnal*sizeof(float));

  mEvalToneMags = new float[mSizeTokenBinAnal];
  memset(mEvalToneMags, 0, mSizeTokenBinAnal*sizeof(float));

  mEnergy = new float[mNumTones]; //For received energy
  mEnergyRatios = new float[mNumTones]; //should be mNumTones!! first fix all the code
  mEnergyStd = new float[mNumTones]; //should be mNumTones!!
  mEnergyDiff = new float[mNumTones]; //should be mNumTones!!

  mEnergyRatiosSorted = new float[mNumTones];
  mEnergyRatiosIdx = new int[mNumTones];

  mReadPosInBlockCircularBuffer = 0;
  mWritePosInBlockCircularBuffer = 0;
  mSizeBlockCircularBuffer = (int)((mSampleRate*Globals::durToken*2.f / (float)mSpectralAnalysis->mHopSize) + 0.5f);

  //mnToleranceFrames = (int)((mSizeBlockCircularBuffer / 2.0) * 0.10 + 0.5f); //10% tolerance
  mnToleranceFrames = (int)((mSizeBlockCircularBuffer / 2.0) * 0.05 + 0.5f); //5% tolerance

  mBlockTokenStatistics = new sTokenProbs[mSizeBlockCircularBuffer];
  memset(mBlockTokenStatistics, 0, mSizeBlockCircularBuffer * sizeof(sTokenProbs));

  mBlockEnergyRatiosTokenIdx = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosTokenIdx,-1,mSizeBlockCircularBuffer*sizeof(int));
  mBlockEnergyStdTokenIdx = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyStdTokenIdx,-1,mSizeBlockCircularBuffer*sizeof(int));
  mBlockEnergyDiffTokenIdx = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyDiffTokenIdx,-1,mSizeBlockCircularBuffer*sizeof(int));

  mBlockEnergyRatiosTokenIdx2 = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosTokenIdx2,-1,mSizeBlockCircularBuffer*sizeof(int));
  mBlockEnergyStdTokenIdx2 = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyStdTokenIdx2,-1,mSizeBlockCircularBuffer*sizeof(int));
  mBlockEnergyDiffTokenIdx2 = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyDiffTokenIdx2,-1,mSizeBlockCircularBuffer*sizeof(int));

  mBlockEnergyRatiosTokenIdx3 = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosTokenIdx3, -1, mSizeBlockCircularBuffer*sizeof(int));

  mBlockEnergyRatiosTokenIdx4 = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosTokenIdx4, -1, mSizeBlockCircularBuffer*sizeof(int));


  //mFirstTokenBinOffsetInBlock = mBinWidth + mSizeNighbBins + 1; //2 binWidth + 1 gap bin + 4 neighbours
  //mBeginBinBlock = mBeginBin - mFirstTokenBinOffsetInBlock;
  //mEndBinBlock = mEndBin + mFirstTokenBinOffsetInBlock;

  mBlockSpecMag = new float*[mSizeBlockCircularBuffer];
  for (int i=0;i<mSizeBlockCircularBuffer;i++)
    mBlockSpecMag[i] = new float[mSpectralAnalysis->mSpecSize];    

  mTokenRepetitions = new int[mNumTokens];
  memset(mTokenRepetitions, 0, mNumTokens*sizeof(int));
  //mLastBlockTokenEnergy = new float[mNumFreqs];
  //memset(mLastBlockTokenEnergy,0,mNumFreqs*sizeof(float));

  mEndStartTokenPosInBlockCircularBuffer = 0;
  mAccumulatedDecodingFrames = 0.0;

  mMessageLength = Globals::numFrontDoorTokens + Globals::numWordTokens + 1; //12 = 2 + 9 + 1 // messagelength=(2 frontdoor + 9 digits) + correction code
  
  mReedSolomon = new ReedSolomon();

  mConfidenceEnergyRatios = 0.f;
  mConfidenceRepetitions = 0.f;
  mConfidenceCorrection = 0.f;
  mConfidence = 0.f;

  mReceivedBeepsVolume = 0.f;


  mDecodedValuesOrig = new int[Globals::numTotalTokens - Globals::numCorrectionTokens]; //For reed solomon statistics
}

Decoder::~Decoder(void)
{
  for (int i=0;i<mSizeBlockCircularBuffer;i++)
    delete [] mBlockSpecMag[i];
  delete [] mBlockSpecMag;

  delete mSpectralAnalysis;

  delete [] mCircularBufferFloat;
  delete [] mAnalBufferFloat;

  delete [] mEvalTokenMags;
  delete [] mEvalToneMags;
    
  delete [] mEnergy;
  delete [] mEnergyRatios;
  delete [] mEnergyStd;
  delete [] mEnergyDiff;

  delete[] mEnergyRatiosSorted;
  delete[] mEnergyRatiosIdx;

  delete[] mBlockTokenStatistics;

  delete [] mBlockEnergyRatiosTokenIdx;
  delete [] mBlockEnergyStdTokenIdx;
  delete [] mBlockEnergyDiffTokenIdx;

  delete [] mBlockEnergyRatiosTokenIdx2;
  delete [] mBlockEnergyStdTokenIdx2;
  delete [] mBlockEnergyDiffTokenIdx2;

  delete[] mBlockEnergyRatiosTokenIdx3;

  delete[] mBlockEnergyRatiosTokenIdx4;

  delete [] mTokenRepetitions;
  //delete [] mLastBlockTokenEnergy;

  delete mReedSolomon;

  delete[] mDecodedValuesOrig; //For reed solomon statistics
}

int Decoder::getSizeFilledFrameCircularBuffer()
{
  int sizeFilled = 0;
  if (mWritePosInFrameCircularBuffer >= mReadPosInFrameCircularBuffer)
  	sizeFilled = mWritePosInFrameCircularBuffer - mReadPosInFrameCircularBuffer;
  else
  	sizeFilled = mWritePosInFrameCircularBuffer + (mSizeFrameCircularBuffer - mReadPosInFrameCircularBuffer);
  return sizeFilled;
}

int Decoder::getSizeFilledBlockCircularBuffer()
{
  int sizeFilled = 0;
  if (mWritePosInBlockCircularBuffer >= mReadPosInBlockCircularBuffer)
  	sizeFilled = mWritePosInBlockCircularBuffer - mReadPosInBlockCircularBuffer;
  else
  	sizeFilled = mWritePosInBlockCircularBuffer + (mSizeBlockCircularBuffer - mReadPosInBlockCircularBuffer);
  return sizeFilled;
}

int Decoder::getSizeFilledBlockCircularBuffer(int mode)
{
  int sizeFilled = 0;
  if (mWritePosInBlockCircularBuffer >= mReadPosInBlockCircularBufferArray[mode])
    sizeFilled = mWritePosInBlockCircularBuffer - mReadPosInBlockCircularBufferArray[mode];
  else
    sizeFilled = mWritePosInBlockCircularBuffer + (mSizeBlockCircularBuffer - mReadPosInBlockCircularBufferArray[mode]);
  return sizeFilled;
}

//Decode audioBuffer to check if begin token is found, we should keep previous buffer to check if token was started in previous
//var mDecoding > 0 when token has been found, once decoding is finished, mDecoding = 0
int Decoder::DecodeAudioBuffer(float *audioBuffer, int size)
{
  return -1;
}

int Decoder::GetDecodedData(char *stringDecoded)
{
  return -1;
}

float Decoder::GetConfidenceError()
{
  //mConfidence = mConfidenceEnergyRatios * mConfidenceCorrection;

  return mConfidenceCorrection;
}

float Decoder::GetConfidenceNoise()
{
  //mConfidence = mConfidenceEnergyRatios * mConfidenceCorrection;

  return mConfidenceEnergyRatios;
}

float Decoder::GetConfidence()
{
  //mConfidence = mConfidenceEnergyRatios * mConfidenceCorrection;

  //calculate confidence:
  //Energy ratios linear between 0(min)-15(max)
  float confidenceEnergyRatiosNorm = 0.f;
  
  if (mConfidenceEnergyRatios <= 0)
    confidenceEnergyRatiosNorm = 0.f;
  else if (mConfidenceEnergyRatios >= 15.f)
    confidenceEnergyRatiosNorm = 1.f;
  else
    confidenceEnergyRatiosNorm = mConfidenceEnergyRatios / 15.f;
  
  return confidenceEnergyRatiosNorm * mConfidenceCorrection;
}

float Decoder::GetReceivedBeepsVolume()
{
  return 20.f * log10(mReceivedBeepsVolume);
}


int Decoder::GetDecodedMode()
{
  //0=AUDIBLE, 1=NONAUDIBLE, 2=HIDDEN, 3=CUSTOM
  return mDecodingMode;
}

float Decoder::GetDecodingBeginFreq()
{
  return 0.f;
}

float Decoder::GetDecodingEndFreq()
{
  return 0.f;
}

int Decoder::GetSpectrum(float *spectrumBuffer)
{
  memcpy(spectrumBuffer, mSpectralAnalysis->mSpecMag, (mSpectralAnalysis->mFftSize / 2) *sizeof(float));
  //for (int i=0;i<mSpectralAnalysis->mFftSize/2;i++)
  //  spectrumBuffer[i] = mSpectralAnalysis->mSpecMag[i];
  
  return mSpectralAnalysis->mFftSize/2;
}


int Decoder::AnalyzeStartTokens(float *audioBuffer)
{ 
  return -1;
}

int Decoder::AnalyzeToken(float *audioBuffer)
{ 
  return -1;
}

int Decoder::ComputeStatsStartTokens()
{
  return -1;
}

int Decoder::ComputeStats()
{
  return -1;
}


// It attenuates the magnitude of those bins in the token that have a larger and stable energy in the last token.
// Input: is a spectogram block of the bins used in the token detection (>3.3kHz)
int Decoder::DeReverbToken(const int nbins, int *freqsBins)
{
  return -1;
}

float Decoder::ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
{
  return -1;
}

float Decoder::ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
{
  return -1;
}







