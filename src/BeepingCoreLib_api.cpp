#include <BeepingConfig.h>
#include <BeepingCoreLib_api.h>
#include <BeepingDebug.h>
#include <DecoderAllMultiTone.h>
#include <DecoderAudibleMultiTone.h>
#include <DecoderNonAudibleMultiTone.h>
#include <EncoderAudibleMultiTone.h>
#include <EncoderNonAudibleMultiTone.h>
#include <Globals.h>
#include <Scheduler.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <string>
#include <string_view>
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
  BEEPING::initBeepingLogger();
  BeepingContext* beeping = new BeepingContext();
  BINFO("BEEPING_Create -> {}", static_cast<void*>(beeping));
  return (void*)beeping;
}

#ifdef __cplusplus
extern "C"
#endif                                         //__cplusplus
    void BEEPING_Destroy(void* beepingObject)  // Destroy beepingObject Object
{
  BINFO("BEEPING_Destroy({})", beepingObject);
  BeepingContext* beeping = static_cast<BeepingContext*>(beepingObject);
  delete beeping;
  BEEPING::shutdownBeepingLogger();
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    const char* BEEPING_GetVersion() {
  // static char version[50] = "BeepingCoreLib version *.*.* [********]";

  BTRACE("BEEPING_GetVersion -> {}", version);
  return version;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int BEEPING_GetVersionInfo(char* versioninfo) {
  snprintf(versioninfo, 100, "%s", version);

  BTRACE("BEEPING_GetVersionInfo -> {} (len={})", versioninfo,
         strlen(versioninfo));
  return strlen(versioninfo);
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_Configure(int mode, float samplingRate, int32_t bufferSize,
                              void* beepingObject) {
  BINFO("BEEPING_Configure mode={} sr={:.1f} buf={} obj={}", mode, samplingRate,
        bufferSize, beepingObject);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  beeping->mSampleRate = samplingRate;
  beeping->mBufferSize = bufferSize;

  // Parametric window size: power-of-2 closest to (sampleRate * 2048/44100).
  // Keeps bin-to-Hz ratio ~constant across all rates for consistent
  // frequency resolution. At 44100→2048, 48000→2048, 96000→4096, 32000→1024.
  {
    float idealWindow = samplingRate * 2048.0f / 44100.0f;
    int winSize = 256;
    while (winSize < static_cast<int>(idealWindow)) winSize *= 2;
    int lowerPow = winSize / 2;
    if (lowerPow >= 256 && (idealWindow - lowerPow) < (winSize - idealWindow)) {
      winSize = lowerPow;
    }
    beeping->mWindowSize = winSize;
  }

  if (beeping->mEncoder) {
    delete beeping->mEncoder;
    beeping->mEncoder = nullptr;
  }

  if (beeping->mDecoder) {
    delete beeping->mDecoder;
    beeping->mDecoder = nullptr;
  }

  beeping->config = compute_config(beeping->mWindowSize, samplingRate);

  if (mode == BEEPING_MODE_AUDIBLE) {
    beeping->mEncoder = new EncoderAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else if (mode == BEEPING_MODE_INAUDIBLE) {
    beeping->mEncoder = new EncoderNonAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderNonAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else if (mode == BEEPING_MODE_ALL) {
    beeping->mEncoder = new EncoderNonAudibleMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
    beeping->mDecoder = new DecoderAllMultiTone(
        beeping->config, samplingRate, bufferSize, beeping->mWindowSize);
  } else {
    // error
    BERROR("BEEPING_Configure -> -1 (unknown mode={})", mode);
    return -1;
  }

  BINFO("BEEPING_Configure -> 0 (windowSize={})", beeping->mWindowSize);
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_SetAudioSignature(int32_t samplesSize,
                                      const float* samplesBuffer,
                                      void* beepingObject) {
  BTRACE("BEEPING_SetAudioSignature samples={} obj={}", samplesSize,
         beepingObject);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  int32_t rc = beeping->mEncoder->SetAudioSignature(samplesSize, samplesBuffer);
  BTRACE("BEEPING_SetAudioSignature -> {}", rc);
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
  BINFO("BEEPING_Encode payload=\"{}\" len={} type={}",
        std::string_view(stringToEncode, size), size, type);
  BeepingContext* beeping = (BeepingContext*)beepingObject;

#ifdef _ANDROID_LOG_
  //  __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo",
  //  "BEEPING_EncodeDataToAudioBuffer %s type %d size %d object %ld",
  //  stringToEncode, type, size, (long)beepingObject );
#endif  //_ANDROID_LOG_

  int32_t result = beeping->mEncoder->EncodeDataToAudioBuffer(
      stringToEncode, type, size, melodyString, melodySize);
  BINFO("BEEPING_Encode -> {} samples", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetEncodedAudioBuffer(float* audioBuffer,
                                          void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  int32_t result = beeping->mEncoder->GetEncodedAudioBuffer(audioBuffer);
  BTRACE("BEEPING_GetEncodedAudioBuffer -> {} samples", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_ResetEncodedAudioBuffer(void* beepingObject) {
  BTRACE("BEEPING_ResetEncodedAudioBuffer obj={}", beepingObject);
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
  BTRACE("BEEPING_DecodeAudioBuffer size={} -> {}", size, result);
  return result;
}

// we should include maxsize?? int32_t maxsize
#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetDecodedData(char* stringDecoded, void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  int32_t rc = beeping->mDecoder->GetDecodedData(stringDecoded);
  BINFO("BEEPING_GetDecodedData -> rc={} data=\"{}\"", rc, stringDecoded);
  return rc;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetConfidenceError(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetConfidenceError();
  BTRACE("BEEPING_GetConfidenceError -> {:.4f}", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetConfidenceNoise(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetConfidenceNoise();
  BTRACE("BEEPING_GetConfidenceNoise -> {:.4f}", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetConfidence(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  // return (beeping->mDecoder->GetConfidence()/2.f)+0.5f;
  float result = beeping->mDecoder->GetConfidence();
  BTRACE("BEEPING_GetConfidence -> {:.4f}", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetReceivedBeepsVolume(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetReceivedBeepsVolume();
  BTRACE("BEEPING_GetReceivedBeepsVolume -> {:.4f}", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetDecodedMode(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  //(AUDIBLE = 0, INAUDIBLE = 1)
  int32_t result = beeping->mDecoder->GetDecodedMode();
  BTRACE("BEEPING_GetDecodedMode -> {}", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetDecodingBeginFreq(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetDecodingBeginFreq();
  BTRACE("BEEPING_GetDecodingBeginFreq -> {:.1f}", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    float BEEPING_GetDecodingEndFreq(void* beepingObject) {
  BeepingContext* beeping = (BeepingContext*)beepingObject;

  float result = beeping->mDecoder->GetDecodingEndFreq();
  BTRACE("BEEPING_GetDecodingEndFreq -> {:.1f}", result);
  return result;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_ComputeBeepSchedule(float duration, float startTime,
                                        float interval, double* outTimestamps,
                                        int32_t maxTimestamps,
                                        int32_t* outCount) {
  BTRACE("BEEPING_ComputeBeepSchedule d={:.3f} s={:.3f} i={:.3f} max={}",
         duration, startTime, interval, maxTimestamps);

  if (duration < BEEPING::kMinBeepWindow || interval <= 0.0f ||
      startTime < 0.0f ||
      (startTime + BEEPING::kMinBeepWindow) > duration + 1e-6f) {
    BERROR(
        "BEEPING_ComputeBeepSchedule -> -2 (invalid params: d={:.3f} s={:.3f} "
        "i={:.3f})",
        duration, startTime, interval);
    return -2;
  }
  if (maxTimestamps > 0 && outTimestamps == nullptr) {
    BERROR("BEEPING_ComputeBeepSchedule -> -1 (null outTimestamps)");
    return -1;
  }

  auto schedule = BEEPING::computeBeepSchedule(duration, startTime, interval);
  const int32_t total = static_cast<int32_t>(schedule.size());
  const int32_t toWrite = std::min(total, maxTimestamps);
  for (int32_t i = 0; i < toWrite; ++i) {
    outTimestamps[i] = schedule[static_cast<size_t>(i)];
  }
  if (outCount) *outCount = total;
  BTRACE("BEEPING_ComputeBeepSchedule -> 0 (count={} written={})", total,
         toWrite);
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetScheduleBufferSize(float duration, void* beepingObject) {
  if (!beepingObject) return -1;
  BeepingContext* beeping = static_cast<BeepingContext*>(beepingObject);
  if (duration <= 0.0f || beeping->mSampleRate <= 0.0f) return -2;
  return static_cast<int32_t>(std::floor(duration * beeping->mSampleRate));
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_EncodeWithSchedule(
        const char* code, int32_t codeSize, int32_t type, const char* melody,
        int32_t melodySize, float duration, float startTime, float interval,
        float beepGainDb, float* outBuffer, int32_t maxSamples,
        int32_t* outSamplesWritten, void* beepingObject) {
  if (!beepingObject) return -3;
  BeepingContext* beeping = static_cast<BeepingContext*>(beepingObject);
  if (!beeping->mEncoder) return -3;
  if (!code || codeSize <= 0) return -2;
  if (duration < BEEPING::kMinBeepWindow || interval <= 0.0f ||
      startTime < 0.0f ||
      (startTime + BEEPING::kMinBeepWindow) > duration + 1e-6f) {
    return -2;
  }

  BINFO(
      "BEEPING_EncodeWithSchedule code=\"{}\" d={:.3f} s={:.3f} i={:.3f} "
      "gainDb={:.2f}",
      std::string_view(code, codeSize), duration, startTime, interval,
      beepGainDb);

  const float sampleRate = beeping->mSampleRate;
  const int32_t requiredSamples =
      static_cast<int32_t>(std::floor(duration * sampleRate));

  if (!outBuffer || maxSamples < requiredSamples) {
    if (outSamplesWritten) *outSamplesWritten = requiredSamples;
    return -1;
  }

  auto schedule = BEEPING::computeBeepSchedule(duration, startTime, interval);

  std::fill(outBuffer, outBuffer + requiredSamples, 0.0f);

  const float clampedDb = std::clamp(beepGainDb, -60.0f, 12.0f);
  const float gainLinear = std::pow(10.0f, clampedDb / 20.0f);

  const int bufferSize = beeping->mBufferSize > 0 ? beeping->mBufferSize : 128;
  std::vector<float> tmpBuf(static_cast<size_t>(bufferSize));

  for (double ts : schedule) {
    const int tsSec = static_cast<int>(ts + 0.5);
    std::string tsB32 = BEEPING::toBase32(tsSec);
    while (tsB32.size() < 4) tsB32.insert(tsB32.begin(), '0');

    std::string payload(code, static_cast<size_t>(codeSize));
    payload += tsB32;

    beeping->mEncoder->EncodeDataToAudioBuffer(
        payload.c_str(), type, static_cast<int>(payload.size()), melody,
        melodySize);

    const int32_t offset = static_cast<int32_t>(std::floor(ts * sampleRate));
    int32_t writePos = offset;
    while (writePos < requiredSamples) {
      const int32_t n = beeping->mEncoder->GetEncodedAudioBuffer(tmpBuf.data());
      if (n <= 0) break;
      const int32_t remaining = requiredSamples - writePos;
      const int32_t toCopy = std::min(n, remaining);
      for (int32_t k = 0; k < toCopy; ++k) {
        outBuffer[writePos + k] = gainLinear * tmpBuf[static_cast<size_t>(k)];
      }
      writePos += n;
      if (n < bufferSize) break;
    }
    beeping->mEncoder->ResetEncodedAudioBuffer();
  }

  if (outSamplesWritten) *outSamplesWritten = requiredSamples;
  BTRACE("BEEPING_EncodeWithSchedule -> 0 (written={} beeps={})",
         requiredSamples, schedule.size());
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_ParseScheduledPayload(const char* payload,
                                          int32_t payloadSize, char* outCode,
                                          int32_t maxCodeSize,
                                          int32_t* outCodeSize,
                                          int32_t* outTimestampSec) {
  const char* codeStart = nullptr;
  int codeSize = 0;
  int tsSec = 0;
  if (!BEEPING::parseScheduledPayload(payload, payloadSize, &codeStart,
                                      &codeSize, &tsSec)) {
    return -2;
  }
  if (outCode) {
    if (maxCodeSize <= codeSize) return -1;
    std::memcpy(outCode, codeStart, static_cast<size_t>(codeSize));
    outCode[codeSize] = '\0';
  }
  if (outCodeSize) *outCodeSize = codeSize;
  if (outTimestampSec) *outTimestampSec = tsSec;
  return 0;
}

#ifdef __cplusplus
extern "C"
#endif  //__cplusplus
    int32_t BEEPING_GetDecodedScheduledPayload(char* outCode,
                                               int32_t maxCodeSize,
                                               int32_t* outCodeSize,
                                               int32_t* outTimestampSec,
                                               void* beepingObject) {
  if (!beepingObject) return 0;
  BeepingContext* beeping = static_cast<BeepingContext*>(beepingObject);
  if (!beeping->mDecoder) return 0;

  // Use a stack buffer large enough for the existing GetDecodedData
  // recommendation (30 chars) plus headroom.
  char raw[64] = {};
  int32_t rc = beeping->mDecoder->GetDecodedData(raw);
  if (rc == 0) return 0;

  const int32_t payloadLen = std::abs(rc);
  int32_t splitRc = BEEPING_ParseScheduledPayload(
      raw, payloadLen, outCode, maxCodeSize, outCodeSize, outTimestampSec);
  if (splitRc != 0) return -10;

  BTRACE("BEEPING_GetDecodedScheduledPayload rc={} -> {}", rc, payloadLen);
  return rc;
}
