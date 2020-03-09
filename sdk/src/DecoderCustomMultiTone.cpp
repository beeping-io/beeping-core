#include "Decoder.h"
#include "DecoderCustomMultiTone.h"
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


using namespace BEEPING;

DecoderCustomMultiTone::DecoderCustomMultiTone(float samplingRate, int buffSize, int windowSize) : Decoder(samplingRate, buffSize, windowSize, Globals::numTokensCustom, Globals::numTonesCustomMultiTone)
{
#ifdef _IOS_LOG_
  ios_log("C++ DecoderCustomMultiTone");
#endif //_IOS_LOG_
  mFreq2Bin = mSpectralAnalysis->mFftSize / mSampleRate;
  mFreqsBins = new int[mNumTones];
  for (int i = 0; i<mNumTones; i++)
    mFreqsBins[i] = (int)(Globals::getToneFromIdxCustomMultiTone(i, mSampleRate, mWindowSize) * mFreq2Bin + .5);

  //Optimize size of block spectrogram (only needed bins in token space range)
  mBeginBin = (int)(Globals::getToneFromIdxCustomMultiTone(0, mSampleRate, mWindowSize) * mFreq2Bin + .5);
  mEndBin = (int)(Globals::getToneFromIdxCustomMultiTone(mNumTones - 1, mSampleRate, mWindowSize) * mFreq2Bin + .5);

  idxFrontDoorToken1 = Globals::getIdxFromChar(Globals::frontDoorTokens[0]);
  idxFrontDoorToken2 = Globals::getIdxFromChar(Globals::frontDoorTokens[1]);

  mIdxs = new int[2];

  mBlockEnergyRatiosMaxToneIdx = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosMaxToneIdx, -1, mSizeBlockCircularBuffer*sizeof(int));

  mBlockEnergyRatiosSecondToneIdx = new int[mSizeBlockCircularBuffer];
  memset(mBlockEnergyRatiosSecondToneIdx, -1, mSizeBlockCircularBuffer*sizeof(int));

  mToneRepetitions = new int[mNumTones];
  memset(mToneRepetitions, 0, mNumTones*sizeof(int));

  idxTonesFrontDoorToken1 = new int[2];
  Globals::getIdxsFromIdxCustomMultiTone(idxFrontDoorToken1, &idxTonesFrontDoorToken1);
  idxTonesFrontDoorToken2 = new int[2];
  Globals::getIdxsFromIdxCustomMultiTone(idxFrontDoorToken2, &idxTonesFrontDoorToken2);

  mDecodingMode = Globals::/*DECODING_MODE::*/DECODING_MODE_CUSTOM; //0=AUDIBLE, 1=NONAUDIBLE, 2=HIDDEN, 3=CUSTOM
}

DecoderCustomMultiTone::~DecoderCustomMultiTone(void)
{
  delete[] mFreqsBins;
  delete[] mIdxs;

  delete[] mBlockEnergyRatiosMaxToneIdx;
  delete[] mBlockEnergyRatiosSecondToneIdx;

  delete[] mToneRepetitions;

  delete[] idxTonesFrontDoorToken1;
  delete[] idxTonesFrontDoorToken2;
}

int DecoderCustomMultiTone::getSizeFilledFrameCircularBuffer()
{
  return Decoder::getSizeFilledFrameCircularBuffer();
}

int DecoderCustomMultiTone::getSizeFilledBlockCircularBuffer()
{
  return Decoder::getSizeFilledBlockCircularBuffer();
}

//Decode audioBuffer to check if begin token is found, we should keep previous buffer to check if token was started in previous
//var mDecoding > 0 when token has been found, once decoding is finished, mDecoding = 0
int DecoderCustomMultiTone::DecodeAudioBuffer(float *audioBuffer, int size)
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
      if (ret == 1)
      {
          mDecoding = 1;
          mConfidenceEnergyRatios = 0.f;
          mConfidenceRepetitions = 0.f;
          mConfidence = 0.f; //reset confidence
          mReceivedBeepsVolume = 0.f;
          // add frontdoor values to the input vector
          mDecodedValues.push_back(idxFrontDoorToken1); //front-door symbols
          mDecodedValues.push_back(idxFrontDoorToken2); //front-door symbols

          return -2; //-2 means start token found
      }
    }
    else if ((mDecoding > 0) && (mDecoding <= Globals::numMessageTokens)) //we already found start token, now start decoding
    {
      int ret = AnalyzeToken(mAnalBufferFloat);
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

int DecoderCustomMultiTone::GetDecodedData(char *stringDecoded)
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


float DecoderCustomMultiTone::GetDecodingBeginFreq()
{
  return Globals::getToneFromIdxCustomMultiTone(0, mSampleRate, mWindowSize);
}

float DecoderCustomMultiTone::GetDecodingEndFreq()
{
  return Globals::getToneFromIdxCustomMultiTone(mNumTones, mSampleRate, mWindowSize); //we use mNumTones despite las tone is in idx:mNumTones-1 but we use some bins above last tone (to avoid reverb)
}


int DecoderCustomMultiTone::GetSpectrum(float *spectrumBuffer)
{
  return Decoder::GetSpectrum(spectrumBuffer);
}


int DecoderCustomMultiTone::AnalyzeStartTokens(float *audioBuffer)
{ //float *magSpectrum, float* realSpectrum, float* imagSpectrum
  mSpectralAnalysis->doFFT(audioBuffer, mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecPhase);
  memcpy(mBlockSpecMag[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer], mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecSize*sizeof(float)); //needed because when starting decoding we need past spectrogram

  //computeStats
  ComputeStatsStartTokens();
 
  //Get sorted mEnergyRatiosIndices
  for (int i = 0; i<mNumTones; i++)
  {
    mEnergyRatiosSorted[i] = mEnergyRatios[i];
    mEnergyRatiosIdx[i] = i;
  }
  int c, d, idx;
  float swap;
  for (c = 0; c<mNumTones - 1; c++)
  {
    for (d = 0; d<mNumTones - c - 1; d++)
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
  mBlockEnergyRatiosTokenIdx[pos]  = idx_a1;
  mBlockEnergyRatiosTokenIdx2[pos] = idx_a2;
  mBlockEnergyRatiosTokenIdx3[pos] = idx_a3;
  mBlockEnergyRatiosTokenIdx4[pos] = idx_a4;

#ifdef _ANDROID_LOG_
  char text[500];
  sprintf(text, "mBlockEnergyRatios[%d]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", mSizeBlockCircularBuffer,
    mBlockEnergyRatiosTokenIdx[0], mBlockEnergyRatiosTokenIdx[1], mBlockEnergyRatiosTokenIdx[2], mBlockEnergyRatiosTokenIdx[3], mBlockEnergyRatiosTokenIdx[4], mBlockEnergyRatiosTokenIdx[5],
    mBlockEnergyRatiosTokenIdx[6], mBlockEnergyRatiosTokenIdx[7], mBlockEnergyRatiosTokenIdx[8], mBlockEnergyRatiosTokenIdx[9], mBlockEnergyRatiosTokenIdx[10],
    mBlockEnergyRatiosTokenIdx[11], mBlockEnergyRatiosTokenIdx[12], mBlockEnergyRatiosTokenIdx[13], mBlockEnergyRatiosTokenIdx[14], mBlockEnergyRatiosTokenIdx[15]);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", text);//Or ANDROID_LOG_INFO, ...
  //__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", pStringDecoded);//Or ANDROID_LOG_INFO, ...
#endif

  mWritePosInBlockCircularBuffer = (mWritePosInBlockCircularBuffer + 1) % (mSizeBlockCircularBuffer);

#ifdef _ANDROID_LOG_
  char text2[150];
  sprintf(text2, "sizeFilled: %d | r:%d w:%d", getSizeFilledBlockCircularBuffer(), mReadPosInBlockCircularBuffer, mWritePosInBlockCircularBuffer);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", text2);//Or ANDROID_LOG_INFO, ...
  //__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", pStringDecoded);//Or ANDROID_LOG_INFO, ...
#endif

  while (getSizeFilledBlockCircularBuffer() >= mSizeBlockCircularBuffer - 1)
  {
    int firstTokenRepetitions = 0;
    for (int i = 0; i<(mSizeBlockCircularBuffer / 2); i++)
    {
      int posIdx = (mReadPosInBlockCircularBuffer + i) % mSizeBlockCircularBuffer;

      if ( (mBlockEnergyRatiosTokenIdx[posIdx] == idxTonesFrontDoorToken1[0])/* ||
           (mBlockEnergyRatiosTokenIdx2[posIdx] == idxTonesFrontDoorToken1[0])*/ )
        firstTokenRepetitions++;

      //if ((mBlockEnergyRatiosTokenIdx[posIdx] == idxTonesFrontDoorToken1[1]) ||
      //  (mBlockEnergyRatiosTokenIdx2[posIdx] == idxTonesFrontDoorToken1[1]))
      //  firstTokenRepetitions++;
    }

    //firstTokenRepetitions = firstTokenRepetitions / 2;

    //Only continue if long firstToken is found
    if (firstTokenRepetitions >= ((mSizeBlockCircularBuffer / 2) - mnToleranceFrames))
    {
      int secondTokenRepetitions = 0;
      for (int i = (mSizeBlockCircularBuffer / 2); i < mSizeBlockCircularBuffer; i++)
      {
        int posIdx = (mReadPosInBlockCircularBuffer + i) % mSizeBlockCircularBuffer;

        if ( (mBlockEnergyRatiosTokenIdx[posIdx] == idxTonesFrontDoorToken2[0]) ||
             (mBlockEnergyRatiosTokenIdx2[posIdx] == idxTonesFrontDoorToken2[0])/* ||
             (mBlockEnergyRatiosTokenIdx3[posIdx] == idxTonesFrontDoorToken2[0]) ||
             (mBlockEnergyRatiosTokenIdx4[posIdx] == idxTonesFrontDoorToken2[0])*/ )
          secondTokenRepetitions++;

        //if ((mBlockEnergyRatiosTokenIdx[posIdx] == idxTonesFrontDoorToken2[1]) ||
        //  (mBlockEnergyRatiosTokenIdx2[posIdx] == idxTonesFrontDoorToken2[1]) ||
        //  (mBlockEnergyRatiosTokenIdx3[posIdx] == idxTonesFrontDoorToken2[1]) ||
        //  (mBlockEnergyRatiosTokenIdx4[posIdx] == idxTonesFrontDoorToken2[1]))
        //  secondTokenRepetitions++;
      }

      //secondTokenRepetitions = secondTokenRepetitions / 2;

      //if (secondTokenRepetitions >= ((mSizeBlockCircularBuffer / 2) - mnToleranceFrames))
      if (secondTokenRepetitions >= ((mSizeBlockCircularBuffer / 2) - (mnToleranceFrames*2))) //allow 10% tolerance for second start token
      {
        //advance buffer read pos
        mReadPosInBlockCircularBuffer = mWritePosInBlockCircularBuffer;
        mEndStartTokenPosInBlockCircularBuffer = mReadPosInBlockCircularBuffer;
        mAccumulatedDecodingFrames = 0.0;
        return 1;
      }
      else
      {
        mReadPosInBlockCircularBuffer = (mReadPosInBlockCircularBuffer + 1) % (mSizeBlockCircularBuffer);
      }
    }
    else
    {
      mReadPosInBlockCircularBuffer = (mReadPosInBlockCircularBuffer + 1) % (mSizeBlockCircularBuffer);
    }

  }

  return 0;
}


int DecoderCustomMultiTone::AnalyzeToken(float *audioBuffer)
{
  //float *magSpectrum, float* realSpectrum, float* imagSpectrum
  mSpectralAnalysis->doFFT(audioBuffer, mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecPhase);

  //fill Spectrum CircularBuffer
  memcpy(mBlockSpecMag[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer], mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecSize*sizeof(float));
  //mBlockSpecMag

  mWritePosInBlockCircularBuffer = (mWritePosInBlockCircularBuffer+1)%(mSizeBlockCircularBuffer);
  
  while (getSizeFilledBlockCircularBuffer() >= (mSizeBlockCircularBuffer/2))
  { //we have a new filled block of (durToken size)

    //Apply dereverb here!
    //DeReverb Spectrogram (mBlockSpecMag)
    //int nbins = (mEndBinBlock-mBeginBinBlock)+1;
    
    //int nbins = (mEndBin-mBeginBin)+1;
    //DeReverbToken(nbins, mFreqsBins); //not needed anymore, now we are varying the base freq between odd and even

    ComputeStats();

    //NEW METHOD
    for (int i = 0; i<mNumTones; i++)
    {
      mToneRepetitions[i] = 0;
    }

    for (int i = 0; i<(mSizeBlockCircularBuffer / 2); i++)
    {
      int s = (mReadPosInBlockCircularBuffer + i) % mSizeBlockCircularBuffer;
      int idx = mBlockEnergyRatiosMaxToneIdx[s];
      mToneRepetitions[idx]++;
      idx = mBlockEnergyRatiosSecondToneIdx[s];
      mToneRepetitions[idx]++;

      //Statistics
      mBlockTokenStatistics[s].idxToken = Globals::getIdxTokenFromIdxsTonesCustomMultiTone(mBlockTokenStatistics[s].idxToneMax, mBlockTokenStatistics[s].idxToneSecond);
      mBlockTokenStatistics[s].energyRatioToken = (mBlockTokenStatistics[s].energyRatioToneMax + mBlockTokenStatistics[s].energyRatioToneSecond) / 2.f;
    }

    int max1 = Globals::maxValueIdx(mToneRepetitions, mNumTones);
    int max2 = Globals::secondValueIdx(mToneRepetitions, mNumTones);
    int max = Globals::getIdxTokenFromIdxsTonesCustomMultiTone(max1, max2);

    //mBlockEnergyRatiosMaxToneIdx[s], mBlockEnergyRatiosSecondToneIdx[s]

    //Calculate probability
    int count = 0;
    float prob = 0.f;
    float prob_combined = 0.f;
    float energy = 0.f;
    for (int i = 0; i < (mSizeBlockCircularBuffer / 2); i++)
    {
      int s = (mReadPosInBlockCircularBuffer + i) % mSizeBlockCircularBuffer;
      if (mBlockTokenStatistics[s].idxToken == max)
      {
        prob += mBlockTokenStatistics[s].energyRatioToken;
        energy += mBlockTokenStatistics[s].energyToken;
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
    int idx = mBlockEnergyRatiosTokenIdx[(mReadPosInBlockCircularBuffer+i)%mSizeBlockCircularBuffer];
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
    mReadPosInBlockCircularBuffer = (mEndStartTokenPosInBlockCircularBuffer + offset) % mSizeBlockCircularBuffer;
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

//This function is called every frame
int DecoderCustomMultiTone::ComputeStatsStartTokens()
{
  //energy mean in the alphabet frequency region
  float energyBlock = 0.f;
  //Old implementation using only instantaneous Spectrum
  for (int i=mBeginBin;i<=mEndBin;i++)
  {
    energyBlock+=mSpectralAnalysis->mSpecMag[i];
  }
  energyBlock = energyBlock / (float)(mEndBin-mBeginBin+1);

  for (int i = 0; i<mNumTones; i++)
  {
    int evalToneBin = mFreqsBins[i];

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
int DecoderCustomMultiTone::ComputeStats()
{
  //energy mean in the alphabet frequency region
  double energyBlock = 0.0;

  //New implementation using Spectrogram
  for (int t=0;t<(mSizeBlockCircularBuffer/2);t++)
  {
    for (int i=mBeginBin;i<=mEndBin;i++)
    {
      energyBlock+=mBlockSpecMag[(mReadPosInBlockCircularBuffer+t)%mSizeBlockCircularBuffer][i];
    }
  }
  energyBlock = energyBlock / ((double)(mEndBin-mBeginBin+1) * (mSizeBlockCircularBuffer/2));

  for (int t = 0; t < (mSizeBlockCircularBuffer / 2); t++)
  {
    for (int i = 0; i < mNumTones; i++)
    {
      int evalToneBin = mFreqsBins[i];

      /*Globals::getIdxsFromIdxNonAudibleMultiTone(i,(int**)&mIdxs);
      //CURRENT TOKEN
      int evalTokenBin1 = (int)(Globals::getToneFromIdxNonAudibleMultiTone(mIdxs[0], mSampleRate, mWindowSize) * mFreq2Bin + .5);
      int evalTokenBin2 = (int)(Globals::getToneFromIdxNonAudibleMultiTone(mIdxs[1], mSampleRate, mWindowSize) * mFreq2Bin + .5);
      */
      //Increment base freq for even and not for odd
      if ((mDecoding % 3) == 1)
      {
        evalToneBin += Globals::nBinsOffsetForCustomMultiTone;
      }
      else if ((mDecoding % 3) == 2)
      {
        evalToneBin += Globals::nBinsOffsetForCustomMultiTone * 2;
      }

      for (int n = 0; n < mSizeTokenBinAnal; n++)
      {
        mEvalToneMags[n] = mBlockSpecMag[(mReadPosInBlockCircularBuffer + t) % mSizeBlockCircularBuffer][evalToneBin - mBinWidth + n];
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

    //mBlockEnergyRatiosTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyRatios;
    int s = (mReadPosInBlockCircularBuffer + t) % mSizeBlockCircularBuffer;
    mBlockEnergyRatiosMaxToneIdx[s] = idx_maxEnergyRatios;
    mBlockEnergyRatiosSecondToneIdx[s] = idx_secondEnergyRatios;

    //mBlockEnergyRatiosTokenIdx2[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyRatios2; //not being used (but don't delete)
    //mBlockEnergyStdTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyStd; //not being used (but don't delete)
    //mBlockEnergyDiffTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyDiff; //not being used (but don't delete)

    //Statistics
    mBlockTokenStatistics[s].idxToneMax = idx_maxEnergyRatios;
    mBlockTokenStatistics[s].idxToneSecond = idx_secondEnergyRatios;
    mBlockTokenStatistics[s].energyRatioToneMax = mEnergyRatios[idx_maxEnergyRatios];
    mBlockTokenStatistics[s].energyRatioToneSecond = mEnergyRatios[idx_secondEnergyRatios];
    mBlockTokenStatistics[s].energyToken = mEnergy[idx_maxEnergyRatios] + mEnergy[idx_secondEnergyRatios];
  }

/*  for (int t = 0; t < (mSizeBlockCircularBuffer / 2); t++)
  {
    int s = (mReadPosInBlockCircularBuffer + t) % mSizeBlockCircularBuffer;

    int energyRatioTokenIdx = Globals::getIdxTokenFromIdxsTonesNonAudibleMultiTone(mBlockEnergyRatiosMaxToneIdx[s], mBlockEnergyRatiosSecondToneIdx[s]);

    mBlockEnergyRatiosTokenIdx[s] = energyRatioTokenIdx;
  }*/

  return 0;
}

float DecoderCustomMultiTone::ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
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
      sumFr += mBlockSpecMag[(mReadPosInBlockCircularBuffer+(mSizeBlockCircularBuffer/2)+i) % mSizeBlockCircularBuffer][j]; // accumulate linear magnitude
    }
    mean += sumFr;
    sumPerFrame.push_back(sumFr);
  }
  
  mean /= mSizeBlockCircularBuffer/2;

  return mean;
}

float DecoderCustomMultiTone::ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
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
      sumFr += mBlockSpecMag[(mReadPosInBlockCircularBuffer+i) % mSizeBlockCircularBuffer][j]; // accumulate linear magnitude
    }
    mean += sumFr;
    sumPerFrame.push_back(sumFr);
  }
  
  mean /= mSizeBlockCircularBuffer/2;

  return mean;
}
