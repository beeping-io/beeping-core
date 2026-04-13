#include <BeepingDebug.h>
#include <EncoderAudible.h>
#include <Globals.h>
#include <ReedSolomon.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <cmath>
#include <iostream>
#include <string_view>

// #include <android/log.h>

using namespace BEEPING;

EncoderAudible::EncoderAudible(const BeepingConfig& config, float samplingRate,
                               int buffsize, int windowSize)
    : Encoder(config, samplingRate, buffsize, windowSize,
              config.numTokensAudible, config.numTokensAudible) {
  //__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "EncoderAudible
  // init" );
  BTRACE("EncoderAudible::ctor sr={:.1f} buf={} win={} tokens={}", samplingRate,
         buffsize, windowSize, config.numTokensAudible);
}

EncoderAudible::~EncoderAudible(void) { BTRACE("EncoderAudible::dtor"); }

int EncoderAudible::EncodeDataToAudioBuffer(const char* stringToEncode,
                                            int type, int size,
                                            const char* melodyString,
                                            int melodySize) {
  BINFO("EncoderAudible::Encode payload=\"{}\" len={} type={}",
        std::string_view(stringToEncode, size), size, type);

  memset(mAudioBufferEncodedString, 0,
         mNumMaxSamplesEncodedString * sizeof(float));

  //  __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo",
  //  "EncodeDataToAudioBuffer %s type %d size %d", stringToEncode, type, size
  //  );

  mNumSamplesEncodedString = 0;

  std::vector<int> digits;

  // Add front-door symbols (start tokens)
  digits.push_back(Globals::getIdxFromChar(
      m_config.frontDoorTokens[0]));  // front-door symbols
  digits.push_back(Globals::getIdxFromChar(
      m_config.frontDoorTokens[1]));  // front-door symbols

  // Add user symbols
  for (int i = 0; i < size; i++) {
    digits.push_back(Globals::getIdxFromChar(stringToEncode[i]));
  }

  //  __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Added
  //  digits");

  // Add check digit
  int checkDigit = 0;
  for (int i = Globals::numFrontDoorTokens;
       i < (Globals::numFrontDoorTokens + Globals::numWordTokens); i++)
    checkDigit += digits[i];
  checkDigit = checkDigit % mNumTokens;

  digits.push_back(checkDigit);

  // Add Reed-Solomon characters http://www.eccpage.com/rs.c
  mReedSolomon->SetMessage(digits);
  mReedSolomon->Encode();
  // get RS code to transmit
  mReedSolomon->GetCode(digits);

  BDEBUG("EncoderAudible::Encode RS encoded {} digits", (int)digits.size());

  for (int i = 0; i < static_cast<int>(digits.size()); i++) {
    //__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Digit %d",
    // i);
    float tailLength = 0.5f;
    float gapLength = 0.1f;  // gap between tokens
    int samplesPerDigit = (int)(mSampleRate * m_config.durToken);
    int samplesForFadeBegin =
        (int)(mSampleRate * m_config.durToken * m_config.durFade);
    int samplesForFadeEnd = (int)(mSampleRate * m_config.durToken * tailLength);
    int samplesForGap = (int)(mSampleRate * m_config.durToken * gapLength);
    float currentFreq =
        Globals::getFreqFromIdxAudible(digits[i], mSampleRate, mWindowSize);
    float currentFreqLoudness =
        Globals::getLoudnessFromIdx(digits[i], m_config.numTokensAudible);
    BTRACE("EncoderAudible::Encode digit[{}]={} freq={:.2f} loudness={:.4f}", i,
           digits[i], currentFreq, currentFreqLoudness);

    for (int t = 0; t < samplesPerDigit; t++) {
      float factor = m_config.tokenAmplitude;

      // if (i==0) samplesForFadeEnd = samplesForFadeBegin; //for first token
      if (i == 0) {
        samplesForFadeEnd = (int)(mSampleRate * m_config.durToken *
                                  (tailLength * 0.5f));  // for first token
        samplesForGap = 0;
      }

      if (t < samplesForFadeBegin)
        factor = factor * (float)t / (float)samplesForFadeBegin;
      else if ((t > (samplesPerDigit - (samplesForFadeEnd + samplesForGap))) &&
               (t < (samplesPerDigit - samplesForGap)))
        factor = factor * (float)((samplesPerDigit - samplesForGap) - t) /
                 (float)(samplesForFadeEnd);
      else if (t >= (samplesPerDigit - samplesForGap))
        factor = 0.f;

      //      __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo",
      //      "Sample %d", t);

      mAudioBufferEncodedString[t + i * samplesPerDigit] =
          currentFreqLoudness * factor *
          sinf((2.f * BeepingConfig::pi * currentFreq) *
               ((float)t / (float)mSampleRate));
    }
    mNumSamplesEncodedString += samplesPerDigit;
  }

  BDEBUG("EncoderAudible::Encode totalSamples={}", mNumSamplesEncodedString);

  mReadIndexEncodedAudioBuffer =
      0;  // New audio has been created, then reset read index

  if (type == 1)  // Add robotic sounds to mAudioBufferEncodedString of size
                  // mNumSamplesEncodedString
  {
    BDEBUG("EncoderAudible::Encode applying robotic sounds");
    // Initialize random seed:
    srand(time(NULL));
    for (int i = 0; i < static_cast<int>(digits.size()); i++) {
      int samplesPerDigit = (int)(mSampleRate * m_config.durToken);
      int samplesForFade =
          (int)(mSampleRate * m_config.durToken * m_config.durFade);

      // Generate a random number:
      int randNumber = rand() % mNumTokens;
      float currentFreq = Globals::getMusicalNoteFromIdx(randNumber);

      float f_start = currentFreq;
      int randOffset = (rand() % 500) - 250;
      float f_end = currentFreq + randOffset;

      for (int t = 0; t < samplesPerDigit; t++) {
        float factor = m_config.tokenAmplitude;

        if (t < samplesForFade)
          factor = factor * (float)t / (float)samplesForFade;
        else if (t > (samplesPerDigit - samplesForFade))
          factor =
              factor * (float)(samplesPerDigit - t) / (float)samplesForFade;

        float delta = t / (float)samplesPerDigit;
        float frequency = f_start + (delta * (f_end - f_start));
        float waveLength = 1.f / frequency;

        float timePos = (float)t / mSampleRate;
        float pos = timePos / waveLength;

        // mAudioBufferEncodedString[t+i*samplesPerDigit] = (0.75f *
        // mAudioBufferEncodedString[t+i*samplesPerDigit]) +
        //                                                  (0.25f * factor *
        //                                                  sin(pos * 2.f *
        //                                                  BeepingConfig::pi));

        float vol = pow(10.f, m_config.synthVolume / 20.f);
        mAudioBufferEncodedString[t + i * samplesPerDigit] =
            (1.f * mAudioBufferEncodedString[t + i * samplesPerDigit]) +
            (vol * factor * sin(pos * 2.f * BeepingConfig::pi));
      }
    }
  } else if (type == 2)  // Add melody to mAudioBufferEncodedString of size
                         // mNumSamplesEncodedString
  {
    BDEBUG("EncoderAudible::Encode applying melody, melodySize={}", melodySize);
    std::vector<int> melodyDigits;

    // Add user symbols
    for (int i = 0; i < melodySize; i++) {
      melodyDigits.push_back(Globals::getIdxFromChar(melodyString[i]));
    }

    int numSamplesMelodyString = 0;
    for (int i = 0; i < static_cast<int>(melodyDigits.size()); i++) {
      int samplesPerDigit = (int)(mSampleRate * m_config.durToken);
      int samplesForFade =
          (int)(mSampleRate * m_config.durToken * m_config.durFade);

      float currentFreq = Globals::getMusicalNoteFromIdx(melodyDigits[i]);

      for (int t = 0; t < samplesPerDigit; t++) {
        float factor = m_config.tokenAmplitude;

        if (t < samplesForFade)
          factor = factor * (float)t / (float)samplesForFade;
        else if (t > (samplesPerDigit - samplesForFade))
          factor =
              factor * (float)(samplesPerDigit - t) / (float)samplesForFade;

        mAudioBufferEncodedString[t + i * samplesPerDigit] =
            (0.75f * mAudioBufferEncodedString[t + i * samplesPerDigit]) +
            (0.25f * factor *
             sinf((2.f * BeepingConfig::pi * currentFreq) *
                  ((float)t / (float)mSampleRate)));
      }
      numSamplesMelodyString += samplesPerDigit;
    }

    if (numSamplesMelodyString > mNumSamplesEncodedString)
      mNumSamplesEncodedString = numSamplesMelodyString;
  }

  return mNumSamplesEncodedString;
}

int EncoderAudible::GetEncodedAudioBuffer(float* audioBuffer) {
  return Encoder::GetEncodedAudioBuffer(audioBuffer);
}

int EncoderAudible::ResetEncodedAudioBuffer() {
  return Encoder::ResetEncodedAudioBuffer();
}
