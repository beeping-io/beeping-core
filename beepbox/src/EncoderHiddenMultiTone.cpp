#include "EncoderHiddenMultiTone.h"
#include "ReedSolomon.h"

#include "Globals.h"
#include <iostream>
#include <cmath>

#include <time.h>

#ifdef LINUX
#include <string.h> //memset()
#include <stdlib.h> //srand()
#endif

#if _MSC_VER >= 1800
#include <algorithm> //for max and min
#endif

//#include <android/log.h>

using namespace BEEPING;

EncoderHiddenMultiTone::EncoderHiddenMultiTone(float samplingRate, int buffsize, int windowSize) : Encoder(samplingRate, buffsize, windowSize, Globals::numTokensHidden, Globals::numTonesHiddenMultiTone)
{
//__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "EncoderHiddenMultiTone init" );
  mCurrentFreqs = new float[2];
  mCurrentFreqsLoudness = new float[2];
}

EncoderHiddenMultiTone::~EncoderHiddenMultiTone(void)
{
  delete [] mCurrentFreqs;
  delete [] mCurrentFreqsLoudness;
}

int EncoderHiddenMultiTone::EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize)
{
  memset(mAudioBufferEncodedString,0,mNumMaxSamplesEncodedString*sizeof(float));

//  __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "EncodeDataToAudioBuffer %s type %d size %d", stringToEncode, type, size );
  
  mNumSamplesEncodedString = 0;

  std::vector<int> digits;
  
  //Add front-door symbols (start tokens)
  digits.push_back(Globals::getIdxFromChar(Globals::frontDoorTokens[0])); //front-door symbols
  digits.push_back(Globals::getIdxFromChar(Globals::frontDoorTokens[1])); //front-door symbols
    
  
  //Add user symbols
  for (int i=0;i<size;i++)
  {
    digits.push_back(Globals::getIdxFromChar(stringToEncode[i]));
  }

//  __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Added digits");
  
  //Add check digit
  int checkDigit = 0;
  for (int i=Globals::numFrontDoorTokens;i<(Globals::numFrontDoorTokens+Globals::numWordTokens);i++)
    checkDigit += digits[i];
  checkDigit = checkDigit % mNumTokens;

  digits.push_back(checkDigit);

  //Add Reed-Solomon characters http://www.eccpage.com/rs.c
  mReedSolomon->SetMessage(digits);
  mReedSolomon->Encode();
  // get RS code to transmit
  mReedSolomon->GetCode(digits);

  double phase1 = 0.0;
  double phase2 = 0.0;

  //for cross-fade
  //double prev_phase1 = 0.0;
  //double prev_phase2 = 0.0;

  double delta_time = 1.0/(double)mSampleRate;

  for (int i = 0; i < digits.size(); i++)
  {
    //__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Digit %d", i);
    float tailLength = 0.1f; //0.5
    float gapLength = 0.05f; //0.1 gap between tokens
    int samplesPerDigit = (int)(mSampleRate * Globals::durToken);
    int samplesForFadeBegin = (int)(mSampleRate * Globals::durToken*Globals::durFade);
    int samplesForFadeEnd = (int)(mSampleRate * Globals::durToken*tailLength);
    int samplesForGap = (int)(mSampleRate * Globals::durToken*gapLength);

    Globals::getFreqsFromIdxHiddenMultiTone(digits[i], mSampleRate, mWindowSize, (float**)&mCurrentFreqs);
    Globals::getLoudnessHiddenMultiToneFromIdx(digits[i], (float**)&mCurrentFreqsLoudness);
    //currentFreqsLoudness[0] = Globals::getLoudnessFromIdx(digits[i]);

    if (i > 1) //after the start tokens
    {
      int n = i - 1; //so first token after the start tokens is n=1
      if ((n % 3) == 1) //Take care, encoder and decoder do not share same odd/even order if you compare mDecoding with digits index
      {
        mCurrentFreqs[0] += Globals::freqOffsetForHiddenMultiTone;
        mCurrentFreqs[1] += Globals::freqOffsetForHiddenMultiTone;
      }
      else if ((n % 3) == 2) //Take care, encoder and decoder do not share same odd/even order if you compare mDecoding with digits index
      {
        mCurrentFreqs[0] += Globals::freqOffsetForHiddenMultiTone * 2;
        mCurrentFreqs[1] += Globals::freqOffsetForHiddenMultiTone * 2;
      }
    }
/*
    //BEGIN LEGATO WITH NEXT NOTE (for cross-fade or chirp between tones)
    float *nextFreqs = new float[2];
    if (i<(20-1))
      Globals::getFreqsFromIdxNonAudibleMultiTone(digits[i+1], mSampleRate, mWindowSize, (float**)&nextFreqs);
    else
    {
      nextFreqs[0] = 18000.f;
      nextFreqs[1] = 18000.f;
    }

    if (i > 1) //after the start tokens
    {
      int n = i; //so first token after the start tokens is n=1
      if ((n % 3) == 1) //Take care, encoder and decoder do not share same odd/even order if you compare mDecoding with digits index
      {
        nextFreqs[0] += Globals::freqOffsetForNonAudibleMultiTone;
        nextFreqs[1] += Globals::freqOffsetForNonAudibleMultiTone;
      }
      else if ((n % 3) == 2) //Take care, encoder and decoder do not share same odd/even order if you compare mDecoding with digits index
      {
        nextFreqs[0] += Globals::freqOffsetForNonAudibleMultiTone * 2;
        nextFreqs[1] += Globals::freqOffsetForNonAudibleMultiTone * 2;
      }
    }
    //END LEGATO WITH NEXT NOTE
*/
    for (int t=0;t<samplesPerDigit;t++)
    {
      float factor = Globals::tokenAmplitude-0.05; // 0.7-0.05 added for second screen

      //if (i==0) samplesForFadeEnd = samplesForFadeBegin; //for first token
      if (i==0)
      {
        factor = factor + 0.05; // increase presence only for first start token 
        samplesForFadeEnd = (int)(mSampleRate * Globals::durToken*(tailLength*0.5f)); //for first token
        samplesForGap = 0;
      }
      if (i==1)
        factor = factor + 0.025; // increase presence also for second start token 

      //if (mCurrentFreqs[0]<19200.f && mCurrentFreqs[1]<19200.f) // avoid distorsion when 2 low freqs together
      //  factor = factor - 0.05;

      phase1 += (2.0*(double)Globals::pi*(double)mCurrentFreqs[0]) * delta_time;//(2.0*(double)Globals::pi*(double)mCurrentFreqs[0])*((double)(t + i*samplesPerDigit) / (double)mSampleRate);
      if (phase1 > 2.0 * Globals::pi) 
        phase1 -= 2.0 * Globals::pi;

      phase2 += (2.0*(double)Globals::pi*(double)mCurrentFreqs[1]) * delta_time;//(2.0*(double)Globals::pi*(double)mCurrentFreqs[0])*((double)(t + i*samplesPerDigit) / (double)mSampleRate);
      if (phase2 > 2.0 * Globals::pi) 
        phase2 -= 2.0 * Globals::pi;

      //SAMPLES DURING FADE-IN
      if (t<samplesForFadeBegin)
      {
        float factorCurrent = factor * (float)t/(float)samplesForFadeBegin;
        //multitone
		    if (i<2)
	      { //only one tone simultaneously for start tones
          mAudioBufferEncodedString[t + i*samplesPerDigit] = (1.0f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1));
	      }
	      else
	      {
          mAudioBufferEncodedString[t + i*samplesPerDigit] = (0.5f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1)) +
                                                             (0.5f * mCurrentFreqsLoudness[1] * factorCurrent * sin(phase2));
		    }
        //unitone
        //mAudioBufferEncodedString[t + i*samplesPerDigit] = 1.f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1);

      } //SAMPLES DURING FADE-OUT
      else if ( (t>(samplesPerDigit-(samplesForFadeEnd+samplesForGap))) && (t<(samplesPerDigit-samplesForGap)) )
      {
        float factorCurrent = factor * (float)((samplesPerDigit-samplesForGap)-t)/(float)(samplesForFadeEnd);

        //multitone
		    if (i<2)
	      { //only one tone simultaneously for start tones
            mAudioBufferEncodedString[t + i*samplesPerDigit] = (1.0f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1));
	      }
	      else
	      {
            mAudioBufferEncodedString[t + i*samplesPerDigit] = (0.5f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1)) +
                                                               (0.5f * mCurrentFreqsLoudness[1] * factorCurrent * sin(phase2));
		    }
        //unitone
        //mAudioBufferEncodedString[t + i*samplesPerDigit] = 1.f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1);

        //for cross-fade
/*
        prev_phase1 += (2.0*(double)Globals::pi*(double)nextFreqs[0]) * delta_time;//(2.0*(double)Globals::pi*(double)mCurrentFreqs[0])*((double)(t + i*samplesPerDigit) / (double)mSampleRate);
        if (prev_phase1 > 2.0 * Globals::pi) 
          prev_phase1 -= 2.0 * Globals::pi;

        prev_phase2 += (2.0*(double)Globals::pi*(double)nextFreqs[1]) * delta_time;//(2.0*(double)Globals::pi*(double)mCurrentFreqs[0])*((double)(t + i*samplesPerDigit) / (double)mSampleRate);
        if (prev_phase2 > 2.0 * Globals::pi) 
          prev_phase2 -= 2.0 * Globals::pi;

        float factorNext = (1.f - (float)((samplesPerDigit-samplesForGap)-t)/(float)(samplesForFadeEnd)) * factor;

        //multitone
        mAudioBufferEncodedString[t + i*samplesPerDigit] = 0.5f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1) +
                                                           0.5f * mCurrentFreqsLoudness[0] * factorNext * sin(prev_phase1) +
                                                           0.5f * mCurrentFreqsLoudness[1] * factorCurrent * sin(phase2) +
                                                           0.5f * mCurrentFreqsLoudness[1] * factorNext * sin(prev_phase2);

        //unitone
        //mAudioBufferEncodedString[t + i*samplesPerDigit] = 1.f * mCurrentFreqsLoudness[0] * factorCurrent * sin(phase1) +
        //                                                   1.f * mCurrentFreqsLoudness[0] * factorNext * sin(prev_phase1);
*/

      } //SAMPLES DURING GAP
      else if (t>=(samplesPerDigit-samplesForGap)) //during the gap
      {
        mAudioBufferEncodedString[t + i*samplesPerDigit] = 0.f;

/*        float factor = Globals::tokenAmplitude * 0.7; // 0.7 added for second screen
       
        float f2_0 = nextFreqs[0];
        float f2_1 = nextFreqs[1];
        
        //mAudioBufferEncodedString[t + i*samplesPerDigit] = 0.5f * factor * 3.f * sinf(g_t);

        float time1 = float(t + i*samplesPerDigit) /(float)mSampleRate;
        float time2 = time1 + (float)samplesForGap/(float)mSampleRate;
        float interval = time2 - time1;
        float n_steps = (float)samplesForGap;
        double _i = (float)(t-(samplesPerDigit-samplesForGap));
        double delta = _i / (float)n_steps;
        double _t = interval * delta;
        //double phase = 2.0 * Globals::pi * _t * (mCurrentFreqs[0] + (f2_0 - mCurrentFreqs[0]) * delta / 2.0);
        double phase = 2.0 * Globals::pi * (mCurrentFreqs[0]*_t + ((f2_0 - mCurrentFreqs[0])/(2.f*(time2-time1))) * powf(_t,2.f));//jjaner
        while (phase > 2.0 * Globals::pi) 
          phase -= 2.f * Globals::pi; // optional

        mAudioBufferEncodedString[t + i*samplesPerDigit] = 0.5f * factor * sinf(phase); */

      } //SAMPLES IN THE MIDDLE OF TONE
      else
      { //multitone
	    if (i<2)
	    { //only one tone simultaneously for start tones
          mAudioBufferEncodedString[t + i*samplesPerDigit] = (1.0f * mCurrentFreqsLoudness[0] * factor * sin(phase1));
	    }
	    else
	    {
          mAudioBufferEncodedString[t + i*samplesPerDigit] = (0.5f * mCurrentFreqsLoudness[0] * factor * sin(phase1)) +
                                                             (0.5f * mCurrentFreqsLoudness[1] * factor * sin(phase2));
	    }
        //unitone
        //mAudioBufferEncodedString[t + i*samplesPerDigit] = 1.f * mCurrentFreqsLoudness[0] * factor * sinf(phase1);

      }
      // x (t) = sin (2*pi  ( f1*(t - t1) + ( (f2 -f1) / (2*(t2-t1)))  (t - t1)^2  )


    }
    mNumSamplesEncodedString+=samplesPerDigit;

//    FOR CROSS-FADE
//    delete [] nextFreqs;
    //for cross-fade
    //phase1 = prev_phase1;
    //phase2 = prev_phase2;

  }

  mReadIndexEncodedAudioBuffer = 0; //New audio has been created, then reset read index
  
  if (type == 1) //Add robotic sounds to mAudioBufferEncodedString of size mNumSamplesEncodedString
  {    
    //Initialize random seed:
	  srand(time(NULL));
    for (int i=0;i<digits.size();i++)
    {
      int samplesPerDigit = (int)(mSampleRate * Globals::durToken);
      int samplesForFade = (int)(mSampleRate * Globals::durToken*Globals::durFade);
      
		  //Generate a random number:
		  int randNumber = rand() % mNumTokens;
      float currentFreq = Globals::getMusicalNoteFromIdx(randNumber);
  
      float f_start = currentFreq;
      int randOffset = (rand() % 500) - 250;
      float f_end = currentFreq + randOffset;
      
      for (int t=0;t<samplesPerDigit;t++)
      {
        float factor = Globals::tokenAmplitude;

        if (t<samplesForFade)
          factor = factor * (float)t/(float)samplesForFade;
        else if (t>(samplesPerDigit-samplesForFade))
          factor = factor * (float)(samplesPerDigit-t)/(float)samplesForFade;

        float delta = t / (float)samplesPerDigit;
        float frequency = f_start + (delta * (f_end - f_start));
        float waveLength = 1.f / frequency;
      
        float timePos = (float)t / mSampleRate; 
        float pos = timePos / waveLength;

        //mAudioBufferEncodedString[t+i*samplesPerDigit] = (0.75f * mAudioBufferEncodedString[t+i*samplesPerDigit]) +
        //                                                 (0.25f * factor * sin(pos * 2.f * Globals::pi));
        
        float vol = pow(10.f, Globals::synthVolume / 20.f);
        mAudioBufferEncodedString[t + i*samplesPerDigit] = (1.f * mAudioBufferEncodedString[t + i*samplesPerDigit]) +
                                                           (vol * factor * sin(pos * 2.f * Globals::pi));
      }

    }
  }
  else if (type == 2) //Add melody to mAudioBufferEncodedString of size mNumSamplesEncodedString
  {   
    std::vector<int> melodyDigits;
  
    //Add user symbols
    for (int i=0;i<melodySize;i++)
    {
      melodyDigits.push_back(Globals::getIdxFromChar(melodyString[i]));
    }

    int numSamplesMelodyString = 0;
    for (int i=0;i<melodyDigits.size();i++)
    {
      int samplesPerDigit = (int)(mSampleRate * Globals::durToken);
      int samplesForFade = (int)(mSampleRate * Globals::durToken*Globals::durFade);
      
		  float currentFreq = Globals::getMusicalNoteFromIdx(melodyDigits[i]);

      for (int t=0;t<samplesPerDigit;t++)
      {
        float factor = Globals::tokenAmplitude;

        if (t<samplesForFade)
          factor = factor * (float)t/(float)samplesForFade;
        else if (t>(samplesPerDigit-samplesForFade))
          factor = factor * (float)(samplesPerDigit-t)/(float)samplesForFade;
        
        mAudioBufferEncodedString[t+i*samplesPerDigit] = (0.75f * mAudioBufferEncodedString[t+i*samplesPerDigit]) +
                                                         (0.25f * factor * sinf((2.f*Globals::pi*currentFreq)*((float)t/(float)mSampleRate)));
      }
      numSamplesMelodyString+=samplesPerDigit;
    }

    if (numSamplesMelodyString > mNumSamplesEncodedString)
      mNumSamplesEncodedString = numSamplesMelodyString;
  }

  if (mnAudioSignatureSamples > 0) //Add Audio Signature (mAudioSignature) to mAudioBufferEncodedString of size mNumSamplesEncodedString
  {
    int sizeToFill = mnAudioSignatureSamples;
    if (mnAudioSignatureSamples > mNumSamplesEncodedString)
      sizeToFill = mNumSamplesEncodedString;

    for (int i = 0; i < sizeToFill; i++)
    {
      mAudioBufferEncodedString[i] = std::max(-1.f, std::min(1.f, mAudioBufferEncodedString[i] + mAudioSignature[i]));
    }
  }

  return mNumSamplesEncodedString;
}


int EncoderHiddenMultiTone::GetEncodedAudioBuffer(float *audioBuffer)
{
  return Encoder::GetEncodedAudioBuffer(audioBuffer);
}

int EncoderHiddenMultiTone::ResetEncodedAudioBuffer()
{
  return Encoder::ResetEncodedAudioBuffer();
}

