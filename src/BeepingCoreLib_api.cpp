#include <BeepingConfig.h>
#include <BeepingCoreLib_api.h>
#include <BeepingDebug.h>
#include <DecoderAllMultiTone.h>
#include <DecoderAudible.h>
#include <DecoderAudibleMultiTone.h>
#include <DecoderCustomMultiTone.h>
#include <DecoderHiddenMultiTone.h>
#include <DecoderNonAudible.h>
#include <DecoderNonAudibleMultiTone.h>
#include <EncoderAudible.h>
#include <EncoderAudibleMultiTone.h>
#include <EncoderCustomMultiTone.h>
#include <EncoderHiddenMultiTone.h>
#include <EncoderNonAudible.h>
#include <EncoderNonAudibleMultiTone.h>
#include <Globals.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <vector>

static const char version[100] = "BeepingCoreLib version 1.0.0 [20220426]";

using namespace BEEPING;

class BeepingContext {
 public:
  BeepingContext() : mEncoder(nullptr), mDecoder(nullptr) {
    // will be created on configure()
  }

  ~BeepingContext() {
    delete mEncoder;
    delete mDecoder;
  }

  // public vars
  BeepingConfig config;

  Encoder* mEncoder;
  Decoder* mDecoder;

  float mSampleRate;  // needed?
  int mBufferSize;    // needed?
  int mWindowSize;

 private:
};

#ifdef __cplusplus
extern "C"
#endif                      //__cplusplus
    void* BEEPING_Create()  // Create BeepingCore Object, the returned object
                            // will be passed as parameter to all API functions
{
#ifdef BEEPING_DEBUG_LOG
  BEEPING::BeepingLogger::instance().openSession();
#endif
  BeepingContext* beeping = new BeepingContext();
  BINFO("BEEPING_Create -> %p", static_cast<void*>(beeping));
  return (void*)beeping;
}

#ifdef __cplusplus
extern "C"
#endif                                         //__cplusplus
    void BEEPING_Destroy(void* beepingObject)  // Destroy beepingObject Object
{
  BINFO("BEEPING_Destroy(%p)", beepingObject);
  BeepingContext* beeping = static_cast<BeepingContext*>(beepingObject);
  delete beeping;
#ifdef BEEPING_DEBUG_LOG
  BEEPING::BeepingLogger::instance().closeSession();
#endif
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    const char* BEEPING_GetVersion() {
  // static char version[50] = "BeepingCoreLib version *.*.* [********]";

  BTRACE("BEEPING_GetVersion -> %s", version);
  return version;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int BEEPING_GetVersionInfo(char* versioninfo) {
  snprintf(versioninfo, 100, "%s", version);

  BTRACE("BEEPING_GetVersionInfo -> %s (len=%zu)", versioninfo,
         strlen(versioninfo));
  return strlen(versioninfo);
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_Configure(int mode, float samplingRate, int32_t bufferSize,
                              void* beepingObject) {
  BINFO("BEEPING_Configure mode=%d sr=%.1f buf=%d obj=%p", mode, samplingRate,
        bufferSize, beepingObject);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  beeping->mSampleRate = samplingRate;
  beeping->mBufferSize = bufferSize;

  if (beeping->mSampleRate == 48000.0)
    beeping->mWindowSize = 2048;
  else if (beeping->mSampleRate == 44100.0)
    beeping->mWindowSize = 2048;
  else if (beeping->mSampleRate == 22050.0)  // not valid!!
    beeping->mWindowSize = 1024;
  else if (beeping->mSampleRate == 11050.0)  // not valid!!
    beeping->mWindowSize = 512;
  else  // not tested
    beeping->mWindowSize = 256;

  if (beeping->mEncoder) {
    delete beeping->mEncoder;
    beeping->mEncoder = nullptr;
  }

  if (beeping->mDecoder) {
    delete beeping->mDecoder;
    beeping->mDecoder = nullptr;
  }

  beeping->config = compute_config(beeping->mWindowSize, samplingRate);

  if (mode == /*BEEPING_MODE::*/ BEEPING_MODE_AUDIBLEOLD)  // Audible
  {
    beeping->mEncoder = new EncoderAudible(beeping->config, samplingRate,
                                           bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderAudible(beeping->config, samplingRate,
                                           bufferSize, beeping->mWindowSize);
  } else if (mode ==
             /*BEEPING_MODE::*/ BEEPING_MODE_NONAUDIBLEOLD)  // Non audible
  {
    beeping->mEncoder = new EncoderNonAudible(beeping->config, samplingRate,
                                              bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderNonAudible(beeping->config, samplingRate,
                                              bufferSize, beeping->mWindowSize);
  } else if (mode ==
             /*BEEPING_MODE::*/ BEEPING_MODE_AUDIBLE)  // Audible Multi-Tone
  {
    beeping->mEncoder = new EncoderAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else if (mode == /*BEEPING_MODE::*/ BEEPING_MODE_NONAUDIBLE)  // NonAudible
                                                                  // Multi-Tone
  {
    beeping->mEncoder = new EncoderNonAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderNonAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else if (mode ==
             /*BEEPING_MODE::*/ BEEPING_MODE_HIDDEN)  // Hidden Multi-Tone
  {
    beeping->mEncoder = new EncoderHiddenMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderHiddenMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else if (mode == /*BEEPING_MODE::*/ BEEPING_MODE_ALL)  // All modes decoded
                                                           // simultaneously
  {
    beeping->mEncoder = new EncoderNonAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderAllMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else if (mode == /*BEEPING_MODE::*/ BEEPING_MODE_CUSTOM)  // Custom mode
  {
    beeping->mEncoder = new EncoderCustomMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderCustomMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else {
    // error
    BERROR("BEEPING_Configure -> -1 (unknown mode=%d)", mode);
    return -1;
  }

  BINFO("BEEPING_Configure -> 0 (windowSize=%d)", beeping->mWindowSize);
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_SetAudioSignature(int32_t samplesSize,
                                      const float* samplesBuffer,
                                      void* beepingObject) {
  BTRACE("BEEPING_SetAudioSignature samples=%d obj=%p", samplesSize,
         beepingObject);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  int32_t rc = beeping->mEncoder->SetAudioSignature(samplesSize, samplesBuffer);
  BTRACE("BEEPING_SetAudioSignature -> %d", rc);
  return rc;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_EncodeDataToAudioBuffer(const char* stringToEncode,
                                            int32_t size, int32_t type,
                                            const char* melodyString,
                                            int32_t melodySize,
                                            void* beepingObject) {
  BINFO("BEEPING_Encode payload=\"%.*s\" len=%d type=%d", size, stringToEncode,
        size, type);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

#ifdef _ANDROID_LOG_
  //  __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo",
  //  "BEEPING_EncodeDataToAudioBuffer %s type %d size %d object %ld",
  //  stringToEncode, type, size, (long)beepingObject );
#endif  //_ANDROID_LOG_

  int32_t result = beeping->mEncoder->EncodeDataToAudioBuffer(
      stringToEncode, type, size, melodyString, melodySize);
  BINFO("BEEPING_Encode -> %d samples", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetEncodedAudioBuffer(float* audioBuffer,
                                          void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  int32_t result = beeping->mEncoder->GetEncodedAudioBuffer(audioBuffer);
  BTRACE("BEEPING_GetEncodedAudioBuffer -> %d samples", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_ResetEncodedAudioBuffer(void* beepingObject) {
  BTRACE("BEEPING_ResetEncodedAudioBuffer obj=%p", beepingObject);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  return beeping->mEncoder->ResetEncodedAudioBuffer();
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_DecodeAudioBuffer(float* audioBuffer, int size,
                                      void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  // Decode audioBuffer to check if begin token is found, we should keep
  // previous buffer to check if token was started in previous var mDecoding > 0
  // when token has been found, once decoding is finished, mDecoding = 0
  int32_t result = beeping->mDecoder->DecodeAudioBuffer(audioBuffer, size);
  BTRACE("BEEPING_DecodeAudioBuffer size=%d -> %d", size, result);
  return result;
}

// we should include maxsize?? int32_t maxsize
#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetDecodedData(char* stringDecoded, void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  int32_t rc = beeping->mDecoder->GetDecodedData(stringDecoded);
  BINFO("BEEPING_GetDecodedData -> rc=%d data=\"%s\"", rc, stringDecoded);
  return rc;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetConfidenceError(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetConfidenceError();
  BTRACE("BEEPING_GetConfidenceError -> %.4f", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetConfidenceNoise(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetConfidenceNoise();
  BTRACE("BEEPING_GetConfidenceNoise -> %.4f", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetConfidence(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  // return (beeping->mDecoder->GetConfidence()/2.f)+0.5f;
  float result = beeping->mDecoder->GetConfidence();
  BTRACE("BEEPING_GetConfidence -> %.4f", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetReceivedBeepsVolume(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetReceivedBeepsVolume();
  BTRACE("BEEPING_GetReceivedBeepsVolume -> %.4f", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetDecodedMode(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  //(AUDIBLE = 0, NONAUDIBLE = 1, HIDDEN = 2, CUSTOM = 3)
  int32_t result = beeping->mDecoder->GetDecodedMode();
  BTRACE("BEEPING_GetDecodedMode -> %d", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetSpectrum(float* spectrumBuffer, void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  int32_t result = beeping->mDecoder->GetSpectrum(spectrumBuffer);
  BTRACE("BEEPING_GetSpectrum -> %d bins", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_SetCustomBaseFreq(float baseFreq, int beepsSeparation,
                                      void* beepingObject) {
  BINFO("BEEPING_SetCustomBaseFreq freq=%.1f sep=%d", baseFreq,
        beepsSeparation);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  beeping->config.freqBaseForCustomMultiTone = baseFreq;
  beeping->config.beepsSeparationForCustomMultiTone = beepsSeparation;
  recompute_custom_offsets(beeping->config, beeping->mWindowSize);

  if (beeping->mDecoder && beeping->mDecoder->mDecodingMode ==
                               Globals::DECODING_MODE_CUSTOM)  // Custom mode
  {
    if (beeping->mEncoder) {
      delete beeping->mEncoder;
      beeping->mEncoder = nullptr;
    }

    if (beeping->mDecoder) {
      delete beeping->mDecoder;
      beeping->mDecoder = nullptr;
    }

    beeping->mEncoder =
        new EncoderCustomMultiTone(beeping->config, beeping->mSampleRate,
                                   beeping->mBufferSize, beeping->mWindowSize);
    beeping->mDecoder =
        new DecoderCustomMultiTone(beeping->config, beeping->mSampleRate,
                                   beeping->mBufferSize, beeping->mWindowSize);
  }

  return 0;  // should return real custom freq after quantization
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetDecodingBeginFreq(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetDecodingBeginFreq();
  BTRACE("BEEPING_GetDecodingBeginFreq -> %.1f", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetDecodingEndFreq(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetDecodingEndFreq();
  BTRACE("BEEPING_GetDecodingEndFreq -> %.1f", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_SetSynthMode(int synthMode, void* beepingObject) {
  BDEBUG("BEEPING_SetSynthMode mode=%d", synthMode);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  beeping->config.synthMode = synthMode;

  return 0;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_SetSynthVolume(float synthVolume, void* beepingObject) {
  BDEBUG("BEEPING_SetSynthVolume vol=%.2f", synthVolume);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  beeping->config.synthVolume = synthVolume;

  return 0;
}
