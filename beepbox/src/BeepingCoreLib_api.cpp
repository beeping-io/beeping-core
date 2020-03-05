#include "BeepingCoreLib_api.h"

#include <iostream>
#include <ctime>
#include <cassert>
#include <cstddef>

#include <string.h>
#include <stdio.h>

#include <vector>
#include <algorithm> //for reverse
//#include <map>

#include "EncoderAudible.h"
#include "DecoderAudible.h"
#include "EncoderNonAudible.h"
#include "DecoderNonAudible.h"
#include "EncoderAudibleMultiTone.h"
#include "DecoderAudibleMultiTone.h"
#include "EncoderNonAudibleMultiTone.h"
#include "DecoderNonAudibleMultiTone.h"
#include "EncoderHiddenMultiTone.h"
#include "DecoderHiddenMultiTone.h"
#include "DecoderAllMultiTone.h"
#include "EncoderCustomMultiTone.h"
#include "DecoderCustomMultiTone.h"

#include "Globals.h"

static const char version[100] = "BeepingCoreLib version 0.9.7 [31052017]";


using namespace BEEPING;

class CBeepingCore
{
  public:
    CBeepingCore()
    {
      //will be created on configure()
      //mEncoder = new Encoder(44100,2048); //configure with default params sr, buffsize
      //mDecoder = new Decoder(44100,2048);
    }

    ~CBeepingCore()
    {
      delete mEncoder;
      delete mDecoder;
    }

    //public functions

    //public vars
    
    Encoder *mEncoder;
    Decoder *mDecoder;
    
    float mSampleRate; //needed?
    int mBufferSize; //needed?
    int mWindowSize;
    
  private:

};


#ifdef __cplusplus
extern "C"
#endif //__cplusplus
void *BEEPING_Create() //Create BeepingCore Object, the returned object will be passed as parameter to all API functions
{
  CBeepingCore *beeping = new CBeepingCore();
  beeping->mEncoder = 0;
  beeping->mDecoder = 0;
  return (void*)beeping;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
void BEEPING_Destroy(void *beepingObject) //Destroy beepingObject Object
{
  CBeepingCore* beeping = static_cast<CBeepingCore*>(beepingObject);
  delete beeping;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
const char* BEEPING_GetVersion()
{
  //static char version[50] = "BeepingCoreLib version *.*.* [********]";

	return version;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int BEEPING_GetVersionInfo(char * versioninfo)
{
  sprintf(versioninfo, "%s", version);

  return strlen(versioninfo);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_Configure(int mode, float samplingRate, int32_t bufferSize, void *beepingObject)
{
	CBeepingCore *beeping = (CBeepingCore*)beepingObject;

	beeping->mSampleRate = samplingRate;
	beeping->mBufferSize = bufferSize;
  
  if (beeping->mSampleRate == 48000.0)
    beeping->mWindowSize = 2048;
  else if (beeping->mSampleRate == 44100.0)
    beeping->mWindowSize = 2048;
  else if (beeping->mSampleRate == 22050.0) //not valid!!
    beeping->mWindowSize = 1024;
  else if (beeping->mSampleRate == 11050.0) //not valid!!
    beeping->mWindowSize = 512;
  else //not tested
    beeping->mWindowSize = 256;

  if (beeping->mEncoder)
  {
    delete beeping->mEncoder;
    beeping->mEncoder = 0;
  }

  if (beeping->mDecoder)
  {
    delete beeping->mDecoder;
    beeping->mDecoder = 0;
  }

  if (mode == /*BEEPING_MODE::*/BEEPING_MODE_AUDIBLEOLD) //Audible
  {
    beeping->mEncoder = new EncoderAudible(samplingRate, bufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderAudible(samplingRate, bufferSize, beeping->mWindowSize);
  }
  else if (mode == /*BEEPING_MODE::*/BEEPING_MODE_NONAUDIBLEOLD) //Non audible
  { 
    beeping->mEncoder = new EncoderNonAudible(samplingRate, bufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderNonAudible(samplingRate, bufferSize, beeping->mWindowSize);
  }
  else if (mode == /*BEEPING_MODE::*/BEEPING_MODE_AUDIBLE) //Audible Multi-Tone
  {
    beeping->mEncoder = new EncoderAudibleMultiTone(samplingRate, bufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderAudibleMultiTone(samplingRate, bufferSize, beeping->mWindowSize);
  }
  else if (mode == /*BEEPING_MODE::*/BEEPING_MODE_NONAUDIBLE) //NonAudible Multi-Tone
  {
    beeping->mEncoder = new EncoderNonAudibleMultiTone(samplingRate, bufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderNonAudibleMultiTone(samplingRate, bufferSize, beeping->mWindowSize);
  }
  else if (mode == /*BEEPING_MODE::*/BEEPING_MODE_HIDDEN) //Hidden Multi-Tone
  {
    beeping->mEncoder = new EncoderHiddenMultiTone(samplingRate, bufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderHiddenMultiTone(samplingRate, bufferSize, beeping->mWindowSize);
  }
  else if (mode == /*BEEPING_MODE::*/BEEPING_MODE_ALL) //All modes decoded simultaneously
  {
    beeping->mEncoder = new EncoderNonAudibleMultiTone(samplingRate, bufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderAllMultiTone(samplingRate, bufferSize, beeping->mWindowSize);
  }
  else if (mode == /*BEEPING_MODE::*/BEEPING_MODE_CUSTOM) //Custom mode
  {
    beeping->mEncoder = new EncoderCustomMultiTone(samplingRate, bufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderCustomMultiTone(samplingRate, bufferSize, beeping->mWindowSize);
  }
  else
  {
    //error
    return -1;
  }

	return 0;
}


#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_SetAudioSignature(int32_t samplesSize, const float *samplesBuffer, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mEncoder->SetAudioSignature(samplesSize, samplesBuffer);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_EncodeDataToAudioBuffer(const char *stringToEncode, int32_t size, int32_t type, const char *melodyString, int32_t melodySize, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

#ifdef _ANDROID_LOG_
  //  __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "BEEPING_EncodeDataToAudioBuffer %s type %d size %d object %ld", stringToEncode, type, size, (long)beepingObject );
#endif //_ANDROID_LOG_
  
  return beeping->mEncoder->EncodeDataToAudioBuffer(stringToEncode, type, size, melodyString, melodySize);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_GetEncodedAudioBuffer(float *audioBuffer, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mEncoder->GetEncodedAudioBuffer(audioBuffer);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_ResetEncodedAudioBuffer(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mEncoder->ResetEncodedAudioBuffer();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_DecodeAudioBuffer(float *audioBuffer, int size, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;
  
  //Decode audioBuffer to check if begin token is found, we should keep previous buffer to check if token was started in previous
  //var mDecoding > 0 when token has been found, once decoding is finished, mDecoding = 0
  return beeping->mDecoder->DecodeAudioBuffer(audioBuffer, size);
}

//we should include maxsize?? int32_t maxsize
#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_GetDecodedData(char *stringDecoded, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mDecoder->GetDecodedData(stringDecoded);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float BEEPING_GetConfidenceError(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mDecoder->GetConfidenceError();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float BEEPING_GetConfidenceNoise(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mDecoder->GetConfidenceNoise();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float BEEPING_GetConfidence(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  //return (beeping->mDecoder->GetConfidence()/2.f)+0.5f;
  return beeping->mDecoder->GetConfidence();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float BEEPING_GetReceivedBeepsVolume(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  //return (beeping->mDecoder->GetConfidence()/2.f)+0.5f;
  return beeping->mDecoder->GetReceivedBeepsVolume();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_GetDecodedMode(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  //(AUDIBLE = 0, NONAUDIBLE = 1, HIDDEN = 2, CUSTOM = 3)
  return beeping->mDecoder->GetDecodedMode();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_GetSpectrum(float *spectrumBuffer, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mDecoder->GetSpectrum(spectrumBuffer);
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_SetCustomBaseFreq(float baseFreq, int beepsSeparation, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;
  
  if (beeping->mDecoder && beeping->mDecoder->mDecodingMode == Globals::DECODING_MODE_CUSTOM) //Custom mode
  {
    if (beeping->mEncoder)
    {
      delete beeping->mEncoder;
      beeping->mEncoder = 0;
    }
    
    if (beeping->mDecoder)
    {
      delete beeping->mDecoder;
      beeping->mDecoder = 0;
    }

    Globals::freqBaseForCustomMultiTone = baseFreq;
    Globals::beepsSeparationForCustomMultiTone = beepsSeparation;
    
    beeping->mEncoder = new EncoderCustomMultiTone(beeping->mSampleRate, beeping->mBufferSize, beeping->mWindowSize); //configure with default params sr, buffsize
    beeping->mDecoder = new DecoderCustomMultiTone(beeping->mSampleRate, beeping->mBufferSize, beeping->mWindowSize);
  }
  else
  {
    Globals::freqBaseForCustomMultiTone = baseFreq;
    Globals::beepsSeparationForCustomMultiTone = beepsSeparation;
  }

  return 0; //should return real custom freq after quantization
}


#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float BEEPING_GetDecodingBeginFreq(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mDecoder->GetDecodingBeginFreq();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
float BEEPING_GetDecodingEndFreq(void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  return beeping->mDecoder->GetDecodingEndFreq();
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_SetSynthMode(int synthMode, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  Globals::synthMode = synthMode;

  return 0;
}

#ifdef __cplusplus
extern "C"
#endif //__cplusplus
int32_t BEEPING_SetSynthVolume(float synthVolume, void *beepingObject)
{
  CBeepingCore *beeping = (CBeepingCore*)beepingObject;

  Globals::synthVolume = synthVolume;

  return 0; 
}
