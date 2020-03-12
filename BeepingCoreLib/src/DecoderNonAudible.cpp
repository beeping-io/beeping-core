#include "Decoder.h"
#include "DecoderNonAudible.h"
#include "ReedSolomon.h"

#include <cstdio>
#include <string.h>
#include <math.h>
#include <vector>
#include <numeric> //accumulate

#if _MSC_VER >= 1800
  #include <algorithm>
#endif

#include "Globals.h"

#include "SpectralAnalysis.h"

#ifdef DEBUG_OUTPUT
#include <iostream>
#endif //DEBUG_OUTPUT


using namespace BEEPING;

DecoderNonAudible::DecoderNonAudible(float samplingRate, int buffSize, int windowSize) : Decoder(samplingRate, buffSize, windowSize, Globals::numTokensNonAudible, Globals::numTokensNonAudible)
{
#ifdef _IOS_LOG_
  ios_log("C++ DecoderNonAudible");
#endif //_IOS_LOG_
  mSizeNighbBins = mBinWidth * 2;
  mSizeNeighbTokenBinAnal = mSizeNighbBins * 2; //8 //4 on top and 4 on bottom
  mEvalNeighbTokenMags = new float[mSizeNeighbTokenBinAnal];
  memset(mEvalNeighbTokenMags, 0, mSizeNeighbTokenBinAnal*sizeof(float));


  mFreq2Bin = mSpectralAnalysis->mFftSize / mSampleRate;
  mFreqsBins = new int[mNumTones];
  for (int i=0;i<mNumTones;i++)
    mFreqsBins[i] = (int)(Globals::getFreqFromIdxNonAudible(i,mSampleRate,mWindowSize) * mFreq2Bin + .5);

  //Optimize size of block spectrogram (only needed bins in token space range)
  mBeginBin = (int)(Globals::getFreqFromIdxNonAudible(0, mSampleRate, mWindowSize) * mFreq2Bin + .5);
  mEndBin = (int)(Globals::getFreqFromIdxNonAudible(mNumTokens-1, mSampleRate, mWindowSize) * mFreq2Bin + .5);

  idxFrontDoorToken1 = Globals::getIdxFromChar(Globals::frontDoorTokens[0]);
  idxFrontDoorToken2 = Globals::getIdxFromChar(Globals::frontDoorTokens[1]);

  mDecodingMode = Globals::/*DECODING_MODE::*/DECODING_MODE_NONAUDIBLE; //0=AUDIBLE, 1=NONAUDIBLE, 2=HIDDEN, 3=CUSTOM
}

DecoderNonAudible::~DecoderNonAudible(void)
{
  delete[] mEvalNeighbTokenMags;

  delete[] mFreqsBins;
}

int DecoderNonAudible::getSizeFilledFrameCircularBuffer()
{
  return Decoder::getSizeFilledFrameCircularBuffer();
}

int DecoderNonAudible::getSizeFilledBlockCircularBuffer()
{
  return Decoder::getSizeFilledBlockCircularBuffer();
}

//Decode audioBuffer to check if begin token is found, we should keep previous buffer to check if token was started in previous
//var mDecoding > 0 when token has been found, once decoding is finished, mDecoding = 0
int DecoderNonAudible::DecodeAudioBuffer(float *audioBuffer, int size)
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
      return -3; //-3 means that complete word has been decoded
    }
  }

  return -1;
}

int DecoderNonAudible::GetDecodedData(char *stringDecoded)
{
  int messageOk = 1;
  // init ReedSolomon functions (set message length)

  mReedSolomon->msg_len = mMessageLength; // messagelength=(2 frontdoor + 9/10 digits + 1/0 correction code (if any))

  //save check token
  int checkTokenReceived = mDecodedValues[Globals::numFrontDoorTokens+Globals::numWordTokens];

  // decode ReedSolomon error correction
  mReedSolomon->SetCode(mDecodedValues);
  mReedSolomon->Decode();
  mReedSolomon->GetMessage(mDecodedValues);

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

int DecoderNonAudible::GetSpectrum(float *spectrumBuffer)
{
  return Decoder::GetSpectrum(spectrumBuffer);
}


int DecoderNonAudible::AnalyzeStartTokens(float *audioBuffer)
{ //float *magSpectrum, float* realSpectrum, float* imagSpectrum
  mSpectralAnalysis->doFFT(audioBuffer, mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecPhase);
  memcpy(mBlockSpecMag[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer], mSpectralAnalysis->mSpecMag, mSpectralAnalysis->mSpecSize*sizeof(float)); //needed because when starting decoding we need past spectrogram

  //computeStats
  ComputeStatsStartTokens();
 
  int idx_a = Globals::maxValueIdx(mEnergyRatios,mNumTokens);
  int idx_a2 = Globals::secondValueIdx(mEnergyRatios, mNumTokens);
  //int idx_b = Globals::maxValueIdx(mEnergyStd,Globals::numFreqs); //not being used (but don't delete)
  //int idx_c = Globals::maxValueIdx(mEnergyDiff,Globals::numFreqs); //not being used (but don't delete)
  
	mBlockEnergyRatiosTokenIdx[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer] = idx_a;
  mBlockEnergyRatiosTokenIdx2[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer] = idx_a2;
  //mBlockEnergyStdTokenIdx[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer] = idx_b; //not being used (but don't delete)
  //mBlockEnergyDiffTokenIdx[mWritePosInBlockCircularBuffer % mSizeBlockCircularBuffer] = idx_c; //not being used  (but don't delete)
  
#ifdef _ANDROID_LOG_
  char text[500];
  sprintf(text,"mBlockEnergyRatios[%d]: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",mSizeBlockCircularBuffer,
    mBlockEnergyRatiosTokenIdx[0],mBlockEnergyRatiosTokenIdx[1],mBlockEnergyRatiosTokenIdx[2],mBlockEnergyRatiosTokenIdx[3],mBlockEnergyRatiosTokenIdx[4],mBlockEnergyRatiosTokenIdx[5],
    mBlockEnergyRatiosTokenIdx[6],mBlockEnergyRatiosTokenIdx[7],mBlockEnergyRatiosTokenIdx[8],mBlockEnergyRatiosTokenIdx[9],mBlockEnergyRatiosTokenIdx[10],
    mBlockEnergyRatiosTokenIdx[11],mBlockEnergyRatiosTokenIdx[12],mBlockEnergyRatiosTokenIdx[13],mBlockEnergyRatiosTokenIdx[14],mBlockEnergyRatiosTokenIdx[15]);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", text);//Or ANDROID_LOG_INFO, ...
  //__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", pStringDecoded);//Or ANDROID_LOG_INFO, ...
#endif

  mWritePosInBlockCircularBuffer = (mWritePosInBlockCircularBuffer+1)%(mSizeBlockCircularBuffer);
  
#ifdef _ANDROID_LOG_
  char text2[150];
  sprintf(text2,"sizeFilled: %d | r:%d w:%d",getSizeFilledBlockCircularBuffer(),mReadPosInBlockCircularBuffer,mWritePosInBlockCircularBuffer);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", text2);//Or ANDROID_LOG_INFO, ...
  //__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", pStringDecoded);//Or ANDROID_LOG_INFO, ...
#endif

  while (getSizeFilledBlockCircularBuffer() >= mSizeBlockCircularBuffer-1)
  {
    int nToleranceFrames = (int)((mSizeBlockCircularBuffer/2.0) * 0.10 + 0.5); //10% tolerance
    int nFramesReverb = (int)((mSizeBlockCircularBuffer/2.0) * 0.5 + 0.5); //50% reverb

    int firstTokenRepetitions = 0;
    for (int i=0;i<(mSizeBlockCircularBuffer/2);i++)
    {
      if (mBlockEnergyRatiosTokenIdx[(mReadPosInBlockCircularBuffer+i)%mSizeBlockCircularBuffer] == idxFrontDoorToken1)
        firstTokenRepetitions++;
    }
    
    int secondTokenRepetitions = 0;
    for (int i=(mSizeBlockCircularBuffer/2);i<mSizeBlockCircularBuffer;i++)
    {
      if (mBlockEnergyRatiosTokenIdx[(mReadPosInBlockCircularBuffer+i)%mSizeBlockCircularBuffer] == idxFrontDoorToken2)
        secondTokenRepetitions++;
    }

    int secondTokenRepetitionsAsSecond = 0;
    for (int i=(mSizeBlockCircularBuffer/2);i<mSizeBlockCircularBuffer;i++)
    {
      if (mBlockEnergyRatiosTokenIdx2[(mReadPosInBlockCircularBuffer+i)%mSizeBlockCircularBuffer] == idxFrontDoorToken2)
        secondTokenRepetitionsAsSecond++;
    }

    if ( ( firstTokenRepetitions >= ((mSizeBlockCircularBuffer/2)-nToleranceFrames) ) && 
         ( ( secondTokenRepetitions >= ((mSizeBlockCircularBuffer/2)-(nToleranceFrames+nFramesReverb)) ) ||
           ( (secondTokenRepetitions + secondTokenRepetitionsAsSecond) >= ((mSizeBlockCircularBuffer/2)-(nToleranceFrames)) ) )
       )
    {
        //advance buffer read pos
        mReadPosInBlockCircularBuffer = mWritePosInBlockCircularBuffer;
        mEndStartTokenPosInBlockCircularBuffer = mReadPosInBlockCircularBuffer;
        mAccumulatedDecodingFrames = 0.0;

        return 1;
    }
    else
    {
      mReadPosInBlockCircularBuffer = (mReadPosInBlockCircularBuffer+1)%(mSizeBlockCircularBuffer);
    }
      
  }

  return 0;
}


int DecoderNonAudible::AnalyzeToken(float *audioBuffer)
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
    int nbins = (mEndBin-mBeginBin)+1;
    DeReverbToken(nbins, mFreqsBins);

    ComputeStats();

    for (int i=0;i<mNumTokens;i++)
    {
      mTokenRepetitions[i]=0;
    }

    //remove reverb
    int reverbFrames = (int)( ((float)mSizeBlockCircularBuffer / 2.f) * 0.10f + .5f); //was 0.25
    //int reverbFrames = 0;
    for (int i=reverbFrames;i<(mSizeBlockCircularBuffer/2);i++)
    {
      int idx = mBlockEnergyRatiosTokenIdx[(mReadPosInBlockCircularBuffer+i)%mSizeBlockCircularBuffer];
      mTokenRepetitions[idx]++;
    }
       
    int max = Globals::maxValueIdx(mTokenRepetitions, mNumTokens);

    //TODO: advance read pos (make sure is the correct advance!!
    mAccumulatedDecodingFrames += mSizeBlockCircularBuffer/2.0;
    int offset = (int)(mAccumulatedDecodingFrames+.5);
    mReadPosInBlockCircularBuffer = (mEndStartTokenPosInBlockCircularBuffer+offset)%mSizeBlockCircularBuffer;        
    //compute thresholds based on senitivity parameter. TODO: apply this??
/*    float sensitivity = 0.5; // the higher the more sensivity has the algorithm. [0..-1]
    float ratiosThres = std::min(-18.f, std::max(-26.f, -26.f + (1.f-sensitivity)*12.f)); // range [-26..-18] def -20
    float diffThres = std::min(-6.f, std::max( -30.f, -30.f + (1.f-sensitivity)*24.f)); // range [-13..-7] def -10
    if ( (maxEnergyRatios > ratiosThres) && (maxEnergyDiff > diffThres) )
    {
      printf("not enough energy");
    }*/

#ifdef DEBUG_OUTPUT
    //PRINT DEBUG STATISTICS
    std::cout << "  [DEBUG_OUTPUT] " << "Token: " << max << " Prob: " << mTokenRepetitions[max] << std::endl;
#endif //DEBUG_OUTPUT

    return max; //token found
  }

  return -1;
}


int DecoderNonAudible::ComputeStatsStartTokens()
{
  //energy mean in the alphabet frequency region
  float energyBlock = 0.f;
  //Old implementation using only instantaneous Spectrum
  for (int i=mBeginBin;i<=mEndBin;i++)
  {
    energyBlock+=mSpectralAnalysis->mSpecMag[i];
  }
  energyBlock = energyBlock / (float)(mEndBin-mBeginBin+1);

  for (int i=0;i<mNumTokens;i++)
  {     
    for(int n=0;n<mSizeTokenBinAnal;n++)
    {
      mEvalTokenMags[n] = mSpectralAnalysis->mSpecMag[mFreqsBins[i]-mBinWidth+n];
    }

    double sum = Globals::sum(mEvalTokenMags, mSizeTokenBinAnal);
    double energyToken = sum / mSizeTokenBinAnal;
    
    mEnergyRatios[i] = 20.0*log10((energyToken) / energyBlock); // difference of 3dB

    //not being used (but don't delete)
    /*
    double stdToken = Globals::standard_deviation(mEvalTokenMags,mSizeTokenBinAnal);

    //NEIGHBOURHOOD OF CURRENT TOKEN
    for(int n=0;n<(mSizeNeighbTokenBinAnal/2);n++)
      mEvalNeighbTokenMags[n] = mSpectralAnalysis->mSpecMag[mFreqsBins[i]-(mBinWidth*3+1)+n];
    for(int n=(mSizeNeighbTokenBinAnal/2);n<mSizeNeighbTokenBinAnal;n++)
      mEvalNeighbTokenMags[n] = mSpectralAnalysis->mSpecMag[mFreqsBins[i]+(mBinWidth+2)+(n-mSizeNeighbTokenBinAnal/2)];

    double sumNeighb = Globals::sum(mEvalNeighbTokenMags,mSizeNeighbTokenBinAnal);
    double diffToken = (2.0*sum - sumNeighb) / (mSizeTokenBinAnal + mSizeNeighbTokenBinAnal);
    
    mEnergyStd[i] = stdToken/energyToken; // ratio between standard deviation and energy
    mEnergyDiff[i] = 20.0*log10(std::max(1e-10,diffToken)); // ratio of the difference filter. clip to 0 the difference value
    */
  }
  
  return 0;
}

int DecoderNonAudible::ComputeStats()
{
  //energy mean in the alphabet frequency region
  double energyBlock = 0.0;

  //New implementation using Spectrogram
  for (int t=0;t<(mSizeBlockCircularBuffer/2);t++)
  {
    int idx = mBlockEnergyRatiosTokenIdx[(mReadPosInBlockCircularBuffer+t)%mSizeBlockCircularBuffer];
   
    for (int i=mBeginBin;i<=mEndBin;i++)
    {
      energyBlock+=mBlockSpecMag[(mReadPosInBlockCircularBuffer+t)%mSizeBlockCircularBuffer][i];
    }
  }
  energyBlock = energyBlock / ((double)(mEndBin-mBeginBin+1) * (mSizeBlockCircularBuffer/2));

  for (int t=0;t<(mSizeBlockCircularBuffer/2);t++)
  {
    for (int i=0;i<mNumTokens;i++)
    {
      //CURRENT TOKEN
      int evalTokenBin = (int)(Globals::getFreqFromIdxNonAudible(i,mSampleRate,mWindowSize) * mFreq2Bin + .5);
      for(int n=0;n<mSizeTokenBinAnal;n++)
      {
        mEvalTokenMags[n] = mBlockSpecMag[(mReadPosInBlockCircularBuffer+t)%mSizeBlockCircularBuffer][evalTokenBin-mBinWidth+n];//evalTokenMags.push_back(mSpectralAnalysis->mSpecMag[evalTokenBin-binWidth+n]);
      }

  /*************** REVERB ISSUE CORRECTION ***********************/    
      //if the frequency where the stats are being computed is equal to the last token decoded, reduce the probability of being chosen (reverb issue)

      int size = mDecodedValues.size();
      if (size > 0)
      {
        if (i == mDecodedValues[size-1])
          for(int n=0;n<mSizeTokenBinAnal;n++)
            mEvalTokenMags[n] *= 0.2f;

        if ((size > 1) && (i == mDecodedValues[size-2]))
          for(int n=0;n<mSizeTokenBinAnal;n++)
            mEvalTokenMags[n] *= 0.4f;

        if ((size > 2) && (i == mDecodedValues[size-3]))
          for(int n=0;n<mSizeTokenBinAnal;n++)
            mEvalTokenMags[n] *= 0.7f;
      }
      else if (mDecoding > 0)
      {
        if (i == idxFrontDoorToken2)
          for(int n=0;n<mSizeTokenBinAnal;n++)
            mEvalTokenMags[n] *= 0.2f;
        
        if (i == idxFrontDoorToken1)
          for(int n=0;n<mSizeTokenBinAnal;n++)
            mEvalTokenMags[n] *= 0.4f;
      }
  /***************************************************************/

      double sum = Globals::sum(mEvalTokenMags, mSizeTokenBinAnal);
      double energyToken = sum / mSizeTokenBinAnal;

      mEnergyRatios[i] = 20.0*log10((energyToken) / energyBlock); // difference of 3dB
      
      //not being used (but don't delete)
      /*
      double stdToken = Globals::standard_deviation(mEvalTokenMags,mSizeTokenBinAnal);

      //NEIGHBOURHOOD OF CURRENT TOKEN
      for(int n=0;n<(mSizeNeighbTokenBinAnal/2);n++)
        mEvalNeighbTokenMags[n] = mSpectralAnalysis->mSpecMag[evalTokenBin-(mBinWidth*3+1)+n]; 
      for(int n=(mSizeNeighbTokenBinAnal/2);n<mSizeNeighbTokenBinAnal;n++)
        mEvalNeighbTokenMags[n] = mSpectralAnalysis->mSpecMag[evalTokenBin+(mBinWidth+2)+(n-mSizeNeighbTokenBinAnal/2)];

      double sumNeighb = Globals::sum(mEvalNeighbTokenMags,mSizeNeighbTokenBinAnal);
      
      double diffToken = (2.0*sum - sumNeighb) / (mSizeTokenBinAnal + mSizeNeighbTokenBinAnal);
      
      mEnergyStd[i] = stdToken/energyToken; // ratio between standard deviation and energy
      mEnergyDiff[i] = 20.0*log10(std::max(1e-10,diffToken)); // ratio of the difference filter. clip to 0 the difference value
      */
//////////////////////////////////
//mEnergyRatios[i] = mEnergyDiff[i]; //trying...
    }

    float maxEnergyRatios = Globals::maxValue(mEnergyRatios,mNumTokens);
    //float maxEnergyStd = Globals::maxValue(mEnergyStd,mNumTokens); //not being used (but don't delete)
    //float maxEnergyDiff = Globals::maxValue(mEnergyDiff,mNumTokens); //not being used (but don't delete)

    int idx_maxEnergyRatios = Globals::maxValueIdx(mEnergyRatios, mNumTokens);
    //int idx_maxEnergyRatios2 = Globals::secondValueIdx(mEnergyRatios,mNumTokens); //not being used (but don't delete)
    //int idx_maxEnergyStd = Globals::maxValueIdx(mEnergyStd,mNumTokens); //not being used (but don't delete)
    //int idx_maxEnergyDiff = Globals::maxValueIdx(mEnergyDiff,mNumTokens); //not being used (but don't delete)
    
    mBlockEnergyRatiosTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyRatios;
    //mBlockEnergyRatiosTokenIdx2[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyRatios2; //not being used (but don't delete)
    //mBlockEnergyStdTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyStd; //not being used (but don't delete)
    //mBlockEnergyDiffTokenIdx[(mReadPosInBlockCircularBuffer+t) % mSizeBlockCircularBuffer] = idx_maxEnergyDiff; //not being used (but don't delete)
  }

  return 0;
}


// It attenuates the magnitude of those bins in the token that have a larger and stable energy in the last token.
// Input: is a spectogram block of the bins used in the token detection (>3.3kHz)
int DecoderNonAudible::DeReverbToken(const int nbins, int *freqsBins)
{
  // TODO: check dimensions of the input block data spectrogram!!!!
  
  //----------------------------
  // pre-emphasis ??
  // apply a log scale attenaution for high freqs
  
  // de-reverb
//  float freq2bin = float(fftsize)/fs;
  int i=0, j=0;
//  std::vector<int> freqsBins;
//  for (i=0; i<freqs.size();++i)
//  {
//    freqsBins.push_back(round(freqs[i] * freq2bin)); //
//  }

  //was 2 for audible but now the frequencies are more closer
  int binwidth = 1; // bins taken into account around the center frequency to sum the energy
  
  for (int i=0;i<mNumTones;i++)
  {
    // Values are normalized in time (number of frames)
    mSumPerFrame.clear();
    // evaluate if reverb:
    // 1- check if mean energy in current is -3dB of last token ...
    // 2- check if energy is decaying in current token ...
    //      (mean(sum(evalTokenCur,2)) < 0.707*mean(sum(evalTokenLast,2))) && ...
    //      sum(evalTokenCur(1:2),2) > sum(evalTokenCur(end-1:end),2) ;
    
    float meanCur = ComputeBlockMagSpecSumsCurrentToken(freqsBins[i], binwidth, nbins, mSumPerFrame);
    float diffBeginEndCur = mSumPerFrame[0] + mSumPerFrame[1] - (mSumPerFrame[(mSizeBlockCircularBuffer/2) - 2] + mSumPerFrame[(mSizeBlockCircularBuffer/2) - 1]);
    float meanLast = ComputeBlockMagSpecSumsLastToken(freqsBins[i], binwidth, nbins, mSumPerFrame);
    
    //bool reverbFound = ( (meanCur < 0.707*meanLast) && diffBeginEndCur > 0) ? true : false;
    bool reverbFound = ( (meanCur < powf(10.f, -2.f/20.f)*meanLast) && (diffBeginEndCur > 0)) ? true : false; //-2dB
    
    if (reverbFound)
    { //      % attenuate 20dB
      //      block_out(:, freqsBins(i)-binwidth:freqsBins(i)+binwidth) = 0.1 * curBlock(:, freqsBins(i)-binwidth:freqsBins(i)+binwidth);
      for (int s=0; s<(mSizeBlockCircularBuffer/2); s++) 
      {

        for (j=freqsBins[i]-binwidth; j<freqsBins[i]+binwidth; j++)
        {
          mBlockSpecMag[(mReadPosInBlockCircularBuffer+s) % mSizeBlockCircularBuffer][j] *= 0.05f;
        }
      }
      
    }
    
  }
  
  return 0;
}

float DecoderNonAudible::ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
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

float DecoderNonAudible::ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, std::vector<float> &sumPerFrame)
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







