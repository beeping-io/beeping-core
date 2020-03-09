#include "Decoder.h"
#include "DecoderAllMultiTone.h"
#include "ReedSolomon.h"

#include <cstdio>
#include <string.h>
#include <math.h>
#include <vector>
#include <numeric> //accumulate

#include "Globals.h"

#if _MSC_VER >= 1800
#include <algorithm>
#endif

#include "SpectralAnalysis.h"

#ifdef DEBUG_OUTPUT
#include <iostream>
#endif //DEBUG_OUTPUT

#define NUM_SIMULTANEOUS_DECODING_MODES 3 //CUSTOM is not included

using namespace BEEPING;

DecoderAllMultiTone::DecoderAllMultiTone(float samplingRate, int buffSize, int windowSize) : Decoder(samplingRate, buffSize, windowSize, Globals::numTokensAll, Globals::numTonesAll)
{
#ifdef _IOS_LOG_
  ios_log("C++ DecoderAllMultiTone");
#endif //_IOS_LOG_
  
  idxFrontDoorToken1 = Globals::getIdxFromChar(Globals::frontDoorTokens[0]);
  idxFrontDoorToken2 = Globals::getIdxFromChar(Globals::frontDoorTokens[1]);

  mIdxs = new int[2];

  mBlockEnergyRatiosMaxToneIdx = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosMaxToneIdx, -1, mSizeBlockCircularBuffer * sizeof(int));

  mBlockEnergyRatiosSecondToneIdx = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosSecondToneIdx, -1, mSizeBlockCircularBuffer * sizeof(int));

  mToneRepetitions = new int[mNumTones];
  memset(mToneRepetitions, 0, mNumTones * sizeof(int));

 
  mFreq2Bin = mSpectralAnalysis->mFftSize / mSampleRate;
  
  //ADD member vars for all possible modes (audible, non-audible & hidden)
  mFreqsBinsArray = new int*[NUM_SIMULTANEOUS_DECODING_MODES];

  mBeginBinArray = new int[NUM_SIMULTANEOUS_DECODING_MODES];
  mEndBinArray = new int[NUM_SIMULTANEOUS_DECODING_MODES];

  idxTonesFrontDoorToken1Array = new int*[NUM_SIMULTANEOUS_DECODING_MODES];
  idxTonesFrontDoorToken2Array = new int*[NUM_SIMULTANEOUS_DECODING_MODES];


  mBlockEnergyRatiosTokenIdx1Array = new int*[NUM_SIMULTANEOUS_DECODING_MODES];
  mBlockEnergyRatiosTokenIdx2Array = new int*[NUM_SIMULTANEOUS_DECODING_MODES];
  mBlockEnergyRatiosTokenIdx3Array = new int*[NUM_SIMULTANEOUS_DECODING_MODES];
  mBlockEnergyRatiosTokenIdx4Array = new int*[NUM_SIMULTANEOUS_DECODING_MODES];

  mReadPosInBlockCircularBufferArray = new int[NUM_SIMULTANEOUS_DECODING_MODES];

  mBlockTokenStatisticsArray = new sTokenProbs*[NUM_SIMULTANEOUS_DECODING_MODES];
  
  for (int t = 0; t < NUM_SIMULTANEOUS_DECODING_MODES; t++)
  {
    mReadPosInBlockCircularBufferArray[t] = 0;
    
    mFreqsBinsArray[t] = new int[mNumTones];

    idxTonesFrontDoorToken1Array[t] = new int[2];
    idxTonesFrontDoorToken2Array[t] = new int[2];

    mBlockEnergyRatiosTokenIdx1Array[t] = new int[mSizeBlockCircularBuffer];
    memset(mBlockEnergyRatiosTokenIdx1Array[t], -1, mSizeBlockCircularBuffer * sizeof(int));
    mBlockEnergyRatiosTokenIdx2Array[t] = new int[mSizeBlockCircularBuffer];
    memset(mBlockEnergyRatiosTokenIdx2Array[t], -1, mSizeBlockCircularBuffer * sizeof(int));
    mBlockEnergyRatiosTokenIdx3Array[t] = new int[mSizeBlockCircularBuffer];
    memset(mBlockEnergyRatiosTokenIdx3Array[t], -1, mSizeBlockCircularBuffer * sizeof(int));
    mBlockEnergyRatiosTokenIdx4Array[t] = new int[mSizeBlockCircularBuffer];
    memset(mBlockEnergyRatiosTokenIdx4Array[t], -1, mSizeBlockCircularBuffer * sizeof(int));

	mBlockTokenStatisticsArray[t] = new sTokenProbs[mSizeBlockCircularBuffer];
	memset(mBlockTokenStatisticsArray[t], 0, mSizeBlockCircularBuffer * sizeof(sTokenProbs));

    if (t == Globals::/*DECODING_MODE::*/DECODING_MODE_NONAUDIBLE)
    {
      for (int i = 0; i < mNumTones; i++)
        mFreqsBinsArray[t][i] = (int)(Globals::getToneFromIdxNonAudibleMultiTone(i, mSampleRate, mWindowSize) * mFreq2Bin + .5);
      //Optimize size of block spectrogram (only needed bins in token space range)
      mBeginBinArray[t] = (int)(Globals::getToneFromIdxNonAudibleMultiTone(0, mSampleRate, mWindowSize) * mFreq2Bin + .5);
      mEndBinArray[t] = (int)(Globals::getToneFromIdxNonAudibleMultiTone(mNumTones - 1, mSampleRate, mWindowSize) * mFreq2Bin + .5);

      Globals::getIdxsFromIdxNonAudibleMultiTone(idxFrontDoorToken1, &(idxTonesFrontDoorToken1Array[t]));
      Globals::getIdxsFromIdxNonAudibleMultiTone(idxFrontDoorToken2, &(idxTonesFrontDoorToken2Array[t]));
    }
    else if (t == Globals::/*DECODING_MODE::*/DECODING_MODE_AUDIBLE)
    {
      for (int i = 0; i < mNumTones; i++)
        mFreqsBinsArray[t][i] = (int)(Globals::getToneFromIdxAudibleMultiTone(i, mSampleRate, mWindowSize) * mFreq2Bin + .5);
      //Optimize size of block spectrogram (only needed bins in token space range)
      mBeginBinArray[t] = (int)(Globals::getToneFromIdxAudibleMultiTone(0, mSampleRate, mWindowSize) * mFreq2Bin + .5);
      mEndBinArray[t] = (int)(Globals::getToneFromIdxAudibleMultiTone(mNumTones - 1, mSampleRate, mWindowSize) * mFreq2Bin + .5);

      Globals::getIdxsFromIdxAudibleMultiTone(idxFrontDoorToken1, &(idxTonesFrontDoorToken1Array[t]));
      Globals::getIdxsFromIdxAudibleMultiTone(idxFrontDoorToken2, &(idxTonesFrontDoorToken2Array[t]));
    }
    else if (t == Globals::/*DECODING_MODE::*/DECODING_MODE_HIDDEN)
    {
      for (int i = 0; i < mNumTones; i++)
        mFreqsBinsArray[t][i] = (int)(Globals::getToneFromIdxHiddenMultiTone(i, mSampleRate, mWindowSize) * mFreq2Bin + .5);
      //Optimize size of block spectrogram (only needed bins in token space range)
      mBeginBinArray[t] = (int)(Globals::getToneFromIdxHiddenMultiTone(0, mSampleRate, mWindowSize) * mFreq2Bin + .5);
      mEndBinArray[t] = (int)(Globals::getToneFromIdxHiddenMultiTone(mNumTones - 1, mSampleRate, mWindowSize) * mFreq2Bin + .5);

      Globals::getIdxsFromIdxHiddenMultiTone(idxFrontDoorToken1, &(idxTonesFrontDoorToken1Array[t]));
      Globals::getIdxsFromIdxHiddenMultiTone(idxFrontDoorToken2, &(idxTonesFrontDoorToken2Array[t]));
    }
  }
}

DecoderAllMultiTone::~DecoderAllMultiTone(void)
{
  //delete[] mFreqsBins;
  for (int i=0;i<NUM_SIMULTANEOUS_DECODING_MODES;i++)
    delete[] mFreqsBinsArray[i];
  delete [] mFreqsBinsArray;

  delete [] mBeginBinArray;
  delete [] mEndBinArray;
  
  delete[] mIdxs;

  delete[] mBlockEnergyRatiosMaxToneIdx;
  delete[] mBlockEnergyRatiosSecondToneIdx;

  delete[] mToneRepetitions;

  for (int i = 0; i < NUM_SIMULTANEOUS_DECODING_MODES; i++)
  {
    delete[] idxTonesFrontDoorToken1Array[i];
    delete[] idxTonesFrontDoorToken2Array[i];
  }
  delete[] idxTonesFrontDoorToken1Array;
  delete[] idxTonesFrontDoorToken2Array;
  

  for (int i = 0; i < NUM_SIMULTANEOUS_DECODING_MODES; i++)
  {
    delete [] mBlockEnergyRatiosTokenIdx1Array[i];
    delete[] mBlockEnergyRatiosTokenIdx2Array[i];
    delete[] mBlockEnergyRatiosTokenIdx3Array[i];
    delete[] mBlockEnergyRatiosTokenIdx4Array[i];
  }
  delete[]  mBlockEnergyRatiosTokenIdx1Array;
  delete[]  mBlockEnergyRatiosTokenIdx2Array;
  delete[]  mBlockEnergyRatiosTokenIdx3Array;
  delete[]  mBlockEnergyRatiosTokenIdx4Array;

  delete[] mReadPosInBlockCircularBufferArray;
  //delete[] idxTonesFrontDoorToken1;
  //delete[] idxTonesFrontDoorToken2;
  
  for (int i = 0; i < NUM_SIMULTANEOUS_DECODING_MODES; i++)
  {
    delete [] mBlockTokenStatisticsArray[i];
  }
  delete [] mBlockTokenStatisticsArray;
}

int DecoderAllMultiTone::getSizeFilledFrameCircularBuffer()
{
  return Decoder::getSizeFilledFrameCircularBuffer();
}

int DecoderAllMultiTone::getSizeFilledBlockCircularBuffer()
{
  return Decoder::getSizeFilledBlockCircularBuffer();
}

int DecoderAllMultiTone::getSizeFilledBlockCircularBuffer(int mode)
{
  return Decoder::getSizeFilledBlockCircularBuffer(mode);
}

//Decode audioBuffer to check if begin token is found, we should keep previous buffer to check if token was started in previous
//var mDecoding > 0 when token has been found, once decoding is finished, mDecoding = 0
int DecoderAllMultiTone::DecodeAudioBuffer(float *audioBuffer, int size)
{
  int sizeWindow = mSpectralAnalysis->mWindowSize;

  int i;
  for(i=0; i<size; i++)
  {
	  mCircularBufferFloat[(i+mWritePosInFrameCircularBuffer)%(mSizeFrameCircularBuffer)] = audioBuffer[i];
  }
  mWritePosInFrameCircularBuffer = (size+mWritePosInFrameCircularBuffer)%(mSizeFrameCircularBuffer);
  
  //if enough data filled (mBufferSize), then send to decode library
  
  while (getSizeFilledFrameCircularBuffer() >= sizeWindow)
  {  	//copy from circularBufferFloat to sendBuffer
  	for (i=0;i<sizeWindow;i++)
  		mAnalBufferFloat[i] = mCircularBufferFloat[(i+mReadPosInFrameCircularBuffer)%(mSizeFrameCircularBuffer)];
    //advance readpos (advance hopsize instead of full buffersize)
  	//mReadPosInFrameCircularBuffer = (mReadPosInFrameCircularBuffer+mBufferSize)%(mSizeFrameCircularBuffer);
    mReadPosInFrameCircularBuffer = (mReadPosInFrameCircularBuffer+mSpectralAnalysis->mHopSize)%(mSizeFrameCircularBuffer);

    if (mDecoding == 0)
    {
      int ret = AnalyzeStartTokens(mAnalBufferFloat);
      if (ret >= 1)
      {
          mDecoding = 1;
          mDecodingMode = ret - 1; //0=AUDIBLE, 1=NONAUDIBLE, 2=HIDDEN
		      mConfidenceEnergyRatios = 0.f;
		      mConfidenceRepetitions = 0.f;
          mConfidence = 0.f; //reset confidence
          mReceivedBeepsVolume = 0.f;
          // add frontdoor values to the input vector
          mDecodedValues.push_back(idxFrontDoorToken1); //front-door symbols
          mDecodedValues.push_back(idxFrontDoorToken2); //front-door symbols

#ifdef DEBUG_OUTPUT
          //PRINT DEBUG STATISTICS
          if ((ret-1) == Globals::DECODING_MODE::DECODING_MODE_AUDIBLE)
            std::cout << "  [DEBUG_OUTPUT] " << "Found Audible Start Token" << std::endl;
          else if ((ret - 1) == Globals::DECODING_MODE::DECODING_MODE_NONAUDIBLE)
            std::cout << "  [DEBUG_OUTPUT] " << "Found Non-Audible Start Token" << std::endl;
          else if ((ret - 1) == Globals::DECODING_MODE::DECODING_MODE_HIDDEN)
            std::cout << "  [DEBUG_OUTPUT] " << "Found Hidden Start Token" << std::endl;

#endif //DEBUG_OUTPUT

          return -2; //-2 means start token found
      }
    }
    else if ((mDecoding > 0) && (mDecoding <= Globals::numMessageTokens)) //we already found start token, now start decoding
    {
      int ret = AnalyzeToken(mAnalBufferFloat, mDecodingMode);
      if (ret >= 0)
      {
        mDecodedValues.push_back(ret);
        mDecoding++;
        return ret;
      }
    }
    else if (mDecoding > Globals::numMessageTokens) //we have finished decoding a complete word
    {
      mDecoding = 0;
	    mConfidenceEnergyRatios = mConfidenceEnergyRatios / Globals::numMessageTokens;
	    mConfidenceRepetitions = mConfidenceRepetitions / Globals::numMessageTokens;
	    mConfidenceRepetitions = (mConfidenceRepetitions / 2.f) + 0.5f;
      
      mReceivedBeepsVolume = mReceivedBeepsVolume / Globals::numMessageTokens;

	    return -3; //-3 means that complete word has been decoded
    }
  }

  return -1;
}

int DecoderAllMultiTone::GetDecodedData(char *stringDecoded)
{
  int messageOk = 1;
  // init ReedSolomon functions (set message length)

  mReedSolomon->msg_len = mMessageLength; // messagelength=(2 frontdoor + 9/10 digits + 1/0 correction code (if any))

  //save check token
  int checkTokenReceived = mDecodedValues[Globals::numFrontDoorTokens+Globals::numWordTokens];

  int sizeTokensNoCorrection = Globals::numTotalTokens - Globals::numCorrectionTokens;
  for (int i = 0; i < sizeTokensNoCorrection; i++)
	mDecodedValuesOrig[i] = mDecodedValues[i];

  // decode ReedSolomon error correction
  mReedSolomon->SetCode(mDecodedValues);
  mReedSolomon->Decode();
  mReedSolomon->GetMessage(mDecodedValues);

  int count = 0;
  for (int i = 0; i < sizeTokensNoCorrection; i++)
  {
    if (mDecodedValues[i] == mDecodedValuesOrig[i])
	  count++;
  }
  mConfidenceCorrection = (float)count / (float)sizeTokensNoCorrection;

#ifdef DEBUG_OUTPUT
  //PRINT DEBUG STATISTICS
  std::cout << "                 " << "  [Prob (correction):" << mConfidenceCorrection << "]" << std::endl;
#endif //DEBUG_OUTPUT

  checkTokenReceived = mDecodedValues[Globals::numFrontDoorTokens+Globals::numWordTokens];

  //check if decoded word is ok using check token
  int checkToken = 0;
  for (int i=Globals::numFrontDoorTokens;i<(Globals::numFrontDoorTokens+Globals::numWordTokens);i++)
    checkToken += mDecodedValues[i];
  checkToken = checkToken % mNumTokens;

  if (checkToken != checkTokenReceived)
    messageOk = -1;

  memset(mDecodedString,0,MAX_DECODE_STRING_SIZE*sizeof(char));

  int len = mDecodedValues.size()-1;

  for (int i=Globals::numFrontDoorTokens; i<len; ++i)
  {
      mDecodedString[i-Globals::numFrontDoorTokens] = Globals::getCharFromIdx(mDecodedValues[i]);
  }

  mDecodedString[len-Globals::numFrontDoorTokens] = '\0'; //Set terminating character, although buffer was already initialized with zeros
  strncpy(stringDecoded, mDecodedString, (len-Globals::numFrontDoorTokens)+1); //to include terminating character
  
  //mDecodedString[0] = '\0';
  memset(mDecodedString,0,MAX_DECODE_STRING_SIZE*sizeof(char)); //initialize for next transmission
  mDecodedValues.clear(); //clear decoded values for next transmission

  return (len-Globals::numFrontDoorTokens) * messageOk; //If message is wrong (token check failed) it returns a negative value
}

int DecoderAllMultiTone::GetSpectrum(float *spectrumBuffer)
{
  return Decoder::GetSpectrum(spectrumBuffer);
}


int DecoderAllMultiTone::AnalyzeStartTokens(float *audioBuffer)
{ //float *magSpectrum, float* realSpectrum, float* imagSpectrum
  mSpectralAnalysis->doFFT(audioBuffer, mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecPhase);
  memcpy(mBlockSpecMag[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer], mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecSize*sizeof(float)); //needed because when starting decoding we need past spectrogram

  //computeStats
  for (int t = 0; t < NUM_SIMULTANEOUS_DECODING_MODES; t++)
  {
    //if (t>0) //to avoid advancement while in loop
    //  mReadPosInBlockCircularBuffer = (mReadPosInBlockCircularBuffer - 1 + mSizeBlockCircularBuffer) % (mSizeBlockCircularBuffer);
    ComputeStatsStartTokens(t);
    
    //Get sorted mEnergyRatiosIndices
    for (int i = 0; i < mNumTones; i++)
    {
      mEnergyRatiosSorted[i] = mEnergyRatios[i];
      mEnergyRatiosIdx[i] = i;
    }
    int c, d, idx;
    float swap;
    for (c = 0; c < mNumTones - 1; c++)
    {
      for (d = 0; d < mNumTones - c - 1; d++)
      {
        if (mEnergyRatiosSorted[d] < mEnergyRatiosSorted[d + 1]) // For increasing order use >
        {
          swap = mEnergyRatiosSorted[d];
          mEnergyRatiosSorted[d] = mEnergyRatiosSorted[d + 1];
          mEnergyRatiosSorted[d + 1] = swap;

          idx = mEnergyRatiosIdx[d];
          mEnergyRatiosIdx[d] = mEnergyRatiosIdx[d + 1];
          mEnergyRatiosIdx[d + 1] = idx;
        }
      }
    }

    int idx_a1 = mEnergyRatiosIdx[0];
    int idx_a2 = mEnergyRatiosIdx[1];
    int idx_a3 = mEnergyRatiosIdx[2];
    int idx_a4 = mEnergyRatiosIdx[3];

    int pos = mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer;
    mBlockEnergyRatiosTokenIdx1Array[t][pos] = idx_a1;
    mBlockEnergyRatiosTokenIdx2Array[t][pos] = idx_a2;
    mBlockEnergyRatiosTokenIdx3Array[t][pos] = idx_a3;
    mBlockEnergyRatiosTokenIdx4Array[t][pos] = idx_a4;

#ifdef _ANDROID_LOG_
    char text[500];
    sprintf(text, "mBlockEnergyRatios[%d]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", mSizeBlockCircularBuffer,
      mBlockEnergyRatiosTokenIdx1Array[t][0], mBlockEnergyRatiosTokenIdx1Array[t][1], mBlockEnergyRatiosTokenIdx1Array[t][2], mBlockEnergyRatiosTokenIdx1Array[t][3], mBlockEnergyRatiosTokenIdx1Array[t][4], mBlockEnergyRatiosTokenIdx1Array[t][5],
      mBlockEnergyRatiosTokenIdx1Array[t][6], mBlockEnergyRatiosTokenIdx1Array[t][7], mBlockEnergyRatiosTokenIdx1Array[t][8], mBlockEnergyRatiosTokenIdx1Array[t][9], mBlockEnergyRatiosTokenIdx1Array[t][10],
      mBlockEnergyRatiosTokenIdx1Array[t][11], mBlockEnergyRatiosTokenIdx1Array[t][12], mBlockEnergyRatiosTokenIdx1Array[t][13], mBlockEnergyRatiosTokenIdx1Array[t][14], mBlockEnergyRatiosTokenIdx1Array[t][15]);
    __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", text);//Or ANDROID_LOG_INFO, ...
    //__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", pStringDecoded);//Or ANDROID_LOG_INFO, ...
#endif

    if (t==0) //only the for the first mode
      mWritePosInBlockCircularBuffer = (mWritePosInBlockCircularBuffer + 1) % (mSizeBlockCircularBuffer);

#ifdef _ANDROID_LOG_
    char text2[150];
    sprintf(text2, "sizeFilled: %d | r:%d w:%d", getSizeFilledBlockCircularBuffer(), mReadPosInBlockCircularBuffer, mWritePosInBlockCircularBuffer);
    __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", text2);//Or ANDROID_LOG_INFO, ...
    //__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", pStringDecoded);//Or ANDROID_LOG_INFO, ...
#endif

    while (getSizeFilledBlockCircularBuffer(t) >= mSizeBlockCircularBuffer - 1)
    {
      int firstTokenRepetitions = 0;
      for (int i = 0; i < (mSizeBlockCircularBuffer / 2); i++)
      {
        int posIdx = (mReadPosInBlockCircularBufferArray[t] + i) % mSizeBlockCircularBuffer;

        if ((mBlockEnergyRatiosTokenIdx1Array[t][posIdx] == idxTonesFrontDoorToken1Array[t][0])/* ||
             (mBlockEnergyRatiosTokenIdx2Array[t][posIdx] == idxTonesFrontDoorToken1Array[t][0])*/)
          firstTokenRepetitions++;

        //if ((mBlockEnergyRatiosTokenIdx1Array[t][posIdx] == idxTonesFrontDoorToken1Array[t][1]) ||
        //  (mBlockEnergyRatiosTokenIdx2Array[t][posIdx] == idxTonesFrontDoorToken1Array[t][1]))
        //  firstTokenRepetitions++;
      }

      //firstTokenRepetitions = firstTokenRepetitions / 2;

      //Only continue if long firstToken is found
      if (firstTokenRepetitions >= ((mSizeBlockCircularBuffer / 2) - mnToleranceFrames))
      {
        int secondTokenRepetitions = 0;
        for (int i = (mSizeBlockCircularBuffer / 2); i < mSizeBlockCircularBuffer; i++)
        {
          int posIdx = (mReadPosInBlockCircularBufferArray[t] + i) % mSizeBlockCircularBuffer;

          if ((mBlockEnergyRatiosTokenIdx1Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][0]) ||
            (mBlockEnergyRatiosTokenIdx2Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][0])/* ||
            (mBlockEnergyRatiosTokenIdx3Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][0]) ||
            (mBlockEnergyRatiosTokenIdx4Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][0])*/)
            secondTokenRepetitions++;

          //if ((mBlockEnergyRatiosTokenIdx1Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][1]) ||
          //  (mBlockEnergyRatiosTokenIdx2Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][1]) ||
          //  (mBlockEnergyRatiosTokenIdx3Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][1]) ||
          //  (mBlockEnergyRatiosTokenIdx4Array[t][posIdx] == idxTonesFrontDoorToken2Array[t][1]))
          //  secondTokenRepetitions++;
        }

        //secondTokenRepetitions = secondTokenRepetitions / 2;

        //if (secondTokenRepetitions >= ((mSizeBlockCircularBuffer / 2) - mnToleranceFrames))
        if (secondTokenRepetitions >= ((mSizeBlockCircularBuffer / 2) - (mnToleranceFrames * 2))) //allow 10% tolerance for second start token
        {
          //advance buffer read pos
          mReadPosInBlockCircularBufferArray[t] = mWritePosInBlockCircularBuffer;
          mEndStartTokenPosInBlockCircularBuffer = mReadPosInBlockCircularBufferArray[t];
          mAccumulatedDecodingFrames = 0.0;
          return 1+t;
        }
        else
        {
          //if (t >= (NUM_SIMULTANEOUS_DECODING_MODES-1))
            mReadPosInBlockCircularBufferArray[t] = (mReadPosInBlockCircularBufferArray[t] + 1) % (mSizeBlockCircularBuffer);
        }
      }
      else
      {
        //if (t >= (NUM_SIMULTANEOUS_DECODING_MODES - 1))
          mReadPosInBlockCircularBufferArray[t] = (mReadPosInBlockCircularBufferArray[t] + 1) % (mSizeBlockCircularBuffer);
      }

    }
  }

  return 0;
}


int DecoderAllMultiTone::AnalyzeToken(float *audioBuffer, int mode)
{
  //float *magSpectrum, float* realSpectrum, float* imagSpectrum
  mSpectralAnalysis->doFFT(audioBuffer, mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecPhase);

  //fill Spectrum CircularBuffer
  memcpy(mBlockSpecMag[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer], mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecSize*sizeof(float));
  //mBlockSpecMag

  mWritePosInBlockCircularBuffer = (mWritePosInBlockCircularBuffer+1)%(mSizeBlockCircularBuffer);
  
  while (getSizeFilledBlockCircularBuffer(mode) >= (mSizeBlockCircularBuffer/2))
  { //we have a new filled block of (durToken size)

    //Apply dereverb here!
    //DeReverb Spectrogram (mBlockSpecMag)
    //int nbins = (mEndBinBlock-mBeginBinBlock)+1;
    
    //int nbins = (mEndBin-mBeginBin)+1;
    //DeReverbToken(nbins, mFreqsBins); //not needed anymore, now we are varying the base freq between odd and even

    ComputeStats(mode);

    //NEW METHOD
    for (int i = 0; i<mNumTones; i++)
    {
      mToneRepetitions[i] = 0;
    }

    for (int i = 0; i<(mSizeBlockCircularBuffer / 2); i++)
    {
      int s = (mReadPosInBlockCircularBufferArray[mode] + i) % mSizeBlockCircularBuffer;
      int idx = mBlockEnergyRatiosMaxToneIdx[s];
      mToneRepetitions[idx]++;
      idx = mBlockEnergyRatiosSecondToneIdx[s];
      mToneRepetitions[idx]++;

	    //Statistics
	    int _idx;
	    if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_NONAUDIBLE)
		  _idx = Globals::getIdxTokenFromIdxsTonesNonAudibleMultiTone(mBlockTokenStatisticsArray[mode][s].idxToneMax, mBlockTokenStatisticsArray[mode][s].idxToneSecond);
	    else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_AUDIBLE)
		  _idx = Globals::getIdxTokenFromIdxsTonesAudibleMultiTone(mBlockTokenStatisticsArray[mode][s].idxToneMax, mBlockTokenStatisticsArray[mode][s].idxToneSecond);
	    else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_HIDDEN)
		  _idx = Globals::getIdxTokenFromIdxsTonesHiddenMultiTone(mBlockTokenStatisticsArray[mode][s].idxToneMax, mBlockTokenStatisticsArray[mode][s].idxToneSecond);
	    mBlockTokenStatisticsArray[mode][s].idxToken = _idx;
	    mBlockTokenStatisticsArray[mode][s].energyRatioToken = (mBlockTokenStatisticsArray[mode][s].energyRatioToneMax + mBlockTokenStatisticsArray[mode][s].energyRatioToneSecond) / 2.f;
    }

    int max1 = Globals::maxValueIdx(mToneRepetitions, mNumTones);
    int max2 = Globals::secondValueIdx(mToneRepetitions, mNumTones);
    
    int max = 0;
    if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_NONAUDIBLE)
      max = Globals::getIdxTokenFromIdxsTonesNonAudibleMultiTone(max1, max2);
    else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_AUDIBLE)
      max = Globals::getIdxTokenFromIdxsTonesAudibleMultiTone(max1, max2);
    else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_HIDDEN)
      max = Globals::getIdxTokenFromIdxsTonesHiddenMultiTone(max1, max2);

    //mBlockEnergyRatiosMaxToneIdx[s], mBlockEnergyRatiosSecondToneIdx[s]

	  //Calculate probability
	  int count = 0;
	  float prob = 0.f;
	  float prob_combined = 0.f;
    float energy = 0.f;
	  for (int i = 0; i < (mSizeBlockCircularBuffer / 2); i++)
	  {
		  int s = (mReadPosInBlockCircularBufferArray[mode] + i) % mSizeBlockCircularBuffer;
		  if (mBlockTokenStatisticsArray[mode][s].idxToken == max)
		  {
			  prob += mBlockTokenStatisticsArray[mode][s].energyRatioToken;
        energy += mBlockTokenStatisticsArray[mode][s].energyToken;
			  count++;
		  }
	  }

	  prob_combined = prob / (float)(mSizeBlockCircularBuffer / 2);

	  if (count > 1)
		  prob /= (float)count;

	  //mConfidenceEnergyRatios += prob;
	  mConfidenceEnergyRatios += prob_combined;
    mReceivedBeepsVolume += energy / (float)(mSizeBlockCircularBuffer / 2);

#ifdef DEBUG_OUTPUT
    //PRINT DEBUG STATISTICS
    std::cout << "  [DEBUG_OUTPUT] " << "Token: " << max << " Prob: " << (((float)mToneRepetitions[max1] + (float)mToneRepetitions[max2]) / (float)mSizeBlockCircularBuffer) << " max1:" << mToneRepetitions[max1] << " max2:" << mToneRepetitions[max2] << std::endl;
	  std::cout << "                 " << "  Prob2:" << prob << std::endl;
	  std::cout << "                 " << "  Prob3 (combined):" << prob_combined << std::endl;
    std::cout << "                 " << "  energy:" << 20 * log10(energy / (float)(mSizeBlockCircularBuffer / 2)) << std::endl;
#endif //DEBUG_OUTPUT
    //END NEW METHOD

    //Update statistics
    mConfidenceRepetitions += ((float)mToneRepetitions[max1] + (float)mToneRepetitions[max2]) / (float)mSizeBlockCircularBuffer;

    //OLD METHOD

    /*    for (int i = 0; i<mNumTokens; i++)
    {
    mTokenRepetitions[i] = 0;
    }

    //remove reverb
    //int reverbFrames = (int)( ((float)mSizeBlockCircularBuffer / 2.f) * 0.10f + .5f); //was 0.25
    int reverbFrames = 0;
    for (int i=reverbFrames;i<(mSizeBlockCircularBuffer/2);i++)
    {
    int idx = mBlockEnergyRatiosTokenIdx1Array[t][(mReadPosInBlockCircularBuffer+i)%mSizeBlockCircularBuffer];
    mTokenRepetitions[idx]++;
    }

    int max = Globals::maxValueIdx(mTokenRepetitions, mNumTokens);

    #ifdef DEBUG_OUTPUT
    //PRINT DEBUG STATISTICS
    std::cout << "  [DEBUG_OUTPUT] " << "Token: " << max << " Prob: " << ((float)mTokenRepetitions[max]/((float)mSizeBlockCircularBuffer/2.f)) << "max:" << mTokenRepetitions[max] << std::endl;
    #endif //DEBUG_OUTPUT

    */
    //END OLD METHOD

    //TODO: advance read pos (make sure is the correct advance!!
    mAccumulatedDecodingFrames += mSizeBlockCircularBuffer / 2.0;
    int offset = (int)(mAccumulatedDecodingFrames + .5);
    mReadPosInBlockCircularBufferArray[mode] = (mEndStartTokenPosInBlockCircularBuffer + offset) % mSizeBlockCircularBuffer;
    //compute thresholds based on senitivity parameter. TODO: apply this??
    /*    float sensitivity = 0.5; // the higher the more sensivity has the algorithm. [0..-1]
    float ratiosThres = std::min(-18.f, std::max(-26.f, -26.f + (1.f-sensitivity)*12.f)); // range [-26..-18] def -20
    float diffThres = std::min(-6.f, std::max( -30.f, -30.f + (1.f-sensitivity)*24.f)); // range [-13..-7] def -10
    if ( (maxEnergyRatios > ratiosThres) && (maxEnergyDiff > diffThres) )
    {
    printf("not enough energy");
    }*/

    return max; //token found
  }

  return -1;
}


int DecoderAllMultiTone::ComputeStatsStartTokens(int mode)
{
  //energy mean in the alphabet frequency region
  float energyBlock = 0.f;
  //Old implementation using only instantaneous Spectrum
  for (int i=mBeginBinArray[mode];i<=mEndBinArray[mode];i++)
  {
    energyBlock+=mSpectralAnalysis->mSpecMag[i];
  }
  energyBlock = energyBlock / (float)(mEndBinArray[mode] -mBeginBinArray[mode] +1);

  for (int i = 0; i<mNumTones; i++)
  {
    int evalToneBin = mFreqsBinsArray[mode][i];

    for (int n = 0; n<mSizeTokenBinAnal; n++)
    {
      mEvalToneMags[n] = mSpectralAnalysis->mSpecMag[evalToneBin - mBinWidth + n];
      //mEvalToneMags[n] = mBlockSpecMag[(mReadPosInBlockCircularBuffer + t) % mSizeBlockCircularBuffer][evalToneBin - mBinWidth + n];
    }

    double sum = Globals::sum(mEvalToneMags, mSizeTokenBinAnal);
    double energyTone = sum / mSizeTokenBinAnal;

    mEnergyRatios[i] = 20.0*log10((energyTone) / energyBlock); // difference of 3dB
  }
  
  return 0;
}

//This function is called every token block, so not so frequent (every tokendur ms)
int DecoderAllMultiTone::ComputeStats(int mode)
{
  //energy mean in the alphabet frequency region
  double energyBlock = 0.0;

  //New implementation using Spectrogram
  for (int t=0;t<(mSizeBlockCircularBuffer/2);t++)
  {
    for (int i=mBeginBinArray[mode];i<=mEndBinArray[mode];i++)
    {
      energyBlock+=mBlockSpecMag[(mReadPosInBlockCircularBufferArray[mode] +t)%mSizeBlockCircularBuffer][i];
    }
  }
  energyBlock = energyBlock / ((double)(mEndBinArray[mode] -mBeginBinArray[mode] +1) * (mSizeBlockCircularBuffer/2));

  for (int t = 0; t < (mSizeBlockCircularBuffer / 2); t++)
  {
    for (int i = 0; i < mNumTones; i++)
    {
      int evalToneBin = mFreqsBinsArray[mode][i];

      /*Globals::getIdxsFromIdxNonAudibleMultiTone(i,(int**)&mIdxs);
      //CURRENT TOKEN
      int evalTokenBin1 = (int)(Globals::getToneFromIdxNonAudibleMultiTone(mIdxs[0], mSampleRate, mWindowSize) * mFreq2Bin + .5);
      int evalTokenBin2 = (int)(Globals::getToneFromIdxNonAudibleMultiTone(mIdxs[1], mSampleRate, mWindowSize) * mFreq2Bin + .5);
      */
      //Increment base freq for even and not for odd
      if ((mDecoding % 3) == 1)
      {
        if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_NONAUDIBLE)
          evalToneBin += Globals::nBinsOffsetForNonAudibleMultiTone;
        else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_AUDIBLE)
          evalToneBin += Globals::nBinsOffsetForAudibleMultiTone;
        else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_HIDDEN)
          evalToneBin += Globals::nBinsOffsetForHiddenMultiTone;
      }
      else if ((mDecoding % 3) == 2)
      {
        if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_NONAUDIBLE)
          evalToneBin += Globals::nBinsOffsetForNonAudibleMultiTone * 2;
        else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_AUDIBLE)
          evalToneBin += Globals::nBinsOffsetForAudibleMultiTone * 2;
        else if (mode == Globals::/*DECODING_MODE::*/DECODING_MODE_HIDDEN)
          evalToneBin += Globals::nBinsOffsetForHiddenMultiTone * 2;
      }

      for (int n = 0; n < mSizeTokenBinAnal; n++)
      {
        mEvalToneMags[n] = mBlockSpecMag[(mReadPosInBlockCircularBufferArray[mode] + t) % mSizeBlockCircularBuffer][evalToneBin - mBinWidth + n];
      }

      double sum = Globals::sum(mEvalToneMags, mSizeTokenBinAnal);
      double square_sum = Globals::sum(mEvalToneMags, mSizeTokenBinAnal);
      double energyToken = sum / mSizeTokenBinAnal;

      mEnergy[i] = (square_sum * mSpectralAnalysis->mSpecSize) / (float)(mSizeBlockCircularBuffer / 2);
      mEnergyRatios[i] = 20.0*log10((energyToken) / energyBlock); // difference of 3dB
    }

    //float maxEnergyRatios = Globals::maxValue(mEnergyRatios,mNumTokens);
    //float maxEnergyStd = Globals::maxValue(mEnergyStd,mNumFreqs); //not being used (but don't delete)
    //float maxEnergyDiff = Globals::maxValue(mEnergyDiff,mNumFreqs); //not being used (but don't delete)

    int idx_maxEnergyRatios = Globals::maxValueIdx(mEnergyRatios, mNumTones);
    int idx_secondEnergyRatios = Globals::secondValueIdx(mEnergyRatios, mNumTones);

    //int idx_maxEnergyRatios2 = Globals::secondValueIdx(mEnergyRatios,mNumFreqs); //not being used (but don't delete)
    //int idx_maxEnergyStd = Globals::maxValueIdx(mEnergyStd,mNumFreqs); //not being used (but don't delete)
    //int idx_maxEnergyDiff = Globals::maxValueIdx(mEnergyDiff,mNumFreqs); //not being used (but don't delete)

    //mBlockEnergyRatiosTokenIdx1Array[t][(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyRatios;
    int s = (mReadPosInBlockCircularBufferArray[mode] + t) % mSizeBlockCircularBuffer;
    mBlockEnergyRatiosMaxToneIdx[s] = idx_maxEnergyRatios;
    mBlockEnergyRatiosSecondToneIdx[s] = idx_secondEnergyRatios;

    //mBlockEnergyRatiosTokenIdx2Array[t][(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyRatios2; //not being used (but don't delete)
    //mBlockEnergyStdTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyStd; //not being used (but don't delete)
    //mBlockEnergyDiffTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyDiff; //not being used (but don't delete)  }
  
	  //Statistics
    mBlockTokenStatisticsArray[mode][s].idxToneMax = idx_maxEnergyRatios;
    mBlockTokenStatisticsArray[mode][s].idxToneSecond = idx_secondEnergyRatios;
    mBlockTokenStatisticsArray[mode][s].energyRatioToneMax = mEnergyRatios[idx_maxEnergyRatios];
    mBlockTokenStatisticsArray[mode][s].energyRatioToneSecond = mEnergyRatios[idx_secondEnergyRatios];
    mBlockTokenStatisticsArray[mode][s].energyToken = mEnergy[idx_maxEnergyRatios] + mEnergy[idx_secondEnergyRatios];
  }

/*  for (int t = 0; t < (mSizeBlockCircularBuffer / 2); t++)
  {
    int s = (mReadPosInBlockCircularBufferArray[mode] + t) % mSizeBlockCircularBuffer;

    int energyRatioTokenIdx = Globals::getIdxTokenFromIdxsTonesHiddenMultiTone(mBlockEnergyRatiosMaxToneIdx[s], mBlockEnergyRatiosSecondToneIdx[s]);

    mBlockEnergyRatiosTokenIdx1Array[mode][s] = energyRatioTokenIdx;
  }*/

  return 0;
}


float DecoderAllMultiTone::ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
{
  float mean = 0;
  float sumFr = 0;

  int lbin = std::max(0,midFreqBin-width);
  int rbin = std::min(nbins, midFreqBin+width);
  for (int i=0; i<(mSizeBlockCircularBuffer/2);i++)
  {
    sumFr = 0;
    for (int j=lbin; j<rbin;j++)
    {
      sumFr += mBlockSpecMag[(mReadPosInBlockCircularBuffer +(mSizeBlockCircularBuffer/2)+i) % mSizeBlockCircularBuffer][j]; // accumulate linear magnitude
    }
    mean += sumFr;
    sumPerFrame.push_back(sumFr);
  }
  
  mean /= mSizeBlockCircularBuffer/2;

  return mean;
}

float DecoderAllMultiTone::ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
{
  float mean = 0;
  float sumFr = 0;

  int lbin = std::max(0,midFreqBin-width);
  int rbin = std::min(nbins, midFreqBin+width);
  for (int i=0; i<(mSizeBlockCircularBuffer/2);i++)
  {
    sumFr = 0;
    for (int j=lbin; j<rbin;j++)
    {
      sumFr += mBlockSpecMag[(mReadPosInBlockCircularBuffer +i) % mSizeBlockCircularBuffer][j]; // accumulate linear magnitude
    }
    mean += sumFr;
    sumPerFrame.push_back(sumFr);
  }
  
  mean /= mSizeBlockCircularBuffer/2;

  return mean;
}
