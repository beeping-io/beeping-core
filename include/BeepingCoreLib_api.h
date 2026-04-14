/**
 * @file BeepingCoreLib_api.h
 * @brief Public C API for the BeepingCore library.
 *
 * BeepingCore encodes and decodes arbitrary data over sound using
 * audible or inaudible multi-tone FSK. This header defines the complete
 * C-compatible API that SDK wrappers (Flutter, Android, iOS, Web, etc.)
 * call into.
 *
 * @copyright Copyright (C) Beeping, LLC. Apache-2.0.
 */

#ifndef __BEEPINGCORELIB_API__
#define __BEEPINGCORELIB_API__

#ifndef __APPLE__
#ifdef BEEPING_AS_DLL
#define BEEPING_DLLEXPORT __declspec(dllexport)
#else
#define BEEPING_DLLEXPORT
#endif

#else
#define BEEPING_DLLEXPORT __attribute__((visibility("default")))
#endif

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif  //__cplusplus

/**
 * @brief Encoding / decoding mode selector.
 *
 * Selects the frequency band used for multi-tone FSK encoding.
 * INAUDIBLE adapts its base frequency to fit under Nyquist at lower
 * sample rates.
 */
enum BEEPING_MODE {
  BEEPING_MODE_AUDIBLE = 2,   /**< Audible tones in the 3.3-10 kHz range. */
  BEEPING_MODE_INAUDIBLE = 3, /**< Inaudible tones (17.8-21 kHz at 44.1k+,
                                   adaptive at lower rates). */
  BEEPING_MODE_ALL = 5        /**< Decode Audible + Inaudible simultaneously. */
};

/** @name Lifecycle */
///@{

/**
 * @brief Create a new BeepingCore instance.
 *
 * The returned opaque handle must be passed to all other API functions
 * and released with BEEPING_Destroy().
 *
 * @return Opaque handle, or `nullptr` on allocation failure.
 */
BEEPING_DLLEXPORT void* BEEPING_Create();

/**
 * @brief Destroy a BeepingCore instance.
 *
 * Releases all resources associated with the instance.
 *
 * @param beepingObject Handle from BEEPING_Create().
 */
BEEPING_DLLEXPORT void BEEPING_Destroy(void* beepingObject);

///@}

/** @name Versioning */
///@{

/**
 * @brief Return the library version string.
 * @return Static null-terminated version string (do not free).
 */
BEEPING_DLLEXPORT const char* BEEPING_GetVersion();

/**
 * @brief Copy the library version string into a caller-provided buffer.
 *
 * @param[out] versioninfo Buffer of at least 100 bytes.
 * @return Number of bytes written (excluding terminator).
 */
BEEPING_DLLEXPORT int32_t BEEPING_GetVersionInfo(char* versioninfo);

///@}

/** @name Configuration */
///@{

/**
 * @brief Configure the library for a given mode and sample rate.
 *
 * This must be called after BEEPING_Create() and before any encode/decode.
 * Reconfiguring is allowed — it destroys the previous encoder/decoder and
 * builds new ones.
 *
 * @param mode One of ::BEEPING_MODE.
 * @param samplingRate Sample rate in Hz (e.g. 44100, 48000, 96000).
 * @param bufferSize Size of the audio buffers the caller will exchange.
 * @param beepingObject Handle from BEEPING_Create().
 * @return 0 on success, negative on failure (e.g. unknown mode).
 */
BEEPING_DLLEXPORT int32_t BEEPING_Configure(int mode, float samplingRate,
                                            int32_t bufferSize,
                                            void* beepingObject);

/**
 * @brief Install a custom audio signature to mix with encoded output.
 *
 * The signature is mixed on top of the tones during playback, useful to
 * add a branded sound.
 *
 * @param samplesSize Number of samples in samplesBuffer. Pass 0 and
 *        nullptr to clear a previously-set signature.
 * @param samplesBuffer Float PCM samples (44.1 kHz, mono). Maximum 2 s
 *        (44100 * 2 samples).
 * @param beepingObject Handle from BEEPING_Create().
 * @return 0 on success, negative on failure.
 */
BEEPING_DLLEXPORT int32_t BEEPING_SetAudioSignature(int32_t samplesSize,
                                                    const float* samplesBuffer,
                                                    void* beepingObject);

///@}

/** @name Encoding */
///@{

/**
 * @brief Encode a string payload into an internal audio buffer.
 *
 * The resulting audio must be drained via BEEPING_GetEncodedAudioBuffer().
 *
 * @param stringToEncode Payload characters to encode.
 * @param size Number of characters to encode.
 * @param type 0 = pure tones, 1 = tones + R2D2 sounds, 2 = melody mode.
 * @param melodyString Melody characters (only used when `type == 2`).
 * @param melodySize Length of the melody (0 for type 0 or 1).
 * @param beepingObject Handle from BEEPING_Create().
 * @return Total number of audio samples generated.
 */
BEEPING_DLLEXPORT int32_t BEEPING_EncodeDataToAudioBuffer(
    const char* stringToEncode, int32_t size, int32_t type,
    const char* melodyString, int32_t melodySize, void* beepingObject);

/**
 * @brief Read a chunk of encoded audio into the caller's buffer.
 *
 * Call repeatedly until a return value less than bufferSize is returned.
 *
 * @param[out] audioBuffer Float array of `bufferSize` samples (as passed
 *        to BEEPING_Configure()).
 * @param beepingObject Handle from BEEPING_Create().
 * @return Number of samples written. Zero or less than bufferSize means
 *         the end of the encoded buffer has been reached.
 */
BEEPING_DLLEXPORT int32_t BEEPING_GetEncodedAudioBuffer(float* audioBuffer,
                                                        void* beepingObject);

/**
 * @brief Reset the read index, allowing the encoded buffer to be re-read.
 * @param beepingObject Handle from BEEPING_Create().
 * @return 0 on success, negative on failure.
 */
BEEPING_DLLEXPORT int32_t BEEPING_ResetEncodedAudioBuffer(void* beepingObject);

///@}

/** @name Decoding */
///@{

/**
 * @brief Feed an audio buffer to the decoder.
 *
 * @param audioBuffer Float PCM samples to decode.
 * @param size Number of samples in audioBuffer.
 * @param beepingObject Handle from BEEPING_Create().
 * @return -1 if no data found yet, -2 if start token detected,
 *         -3 if a complete word has been decoded, or a positive token
 *         index if a single token was decoded.
 */
BEEPING_DLLEXPORT int32_t BEEPING_DecodeAudioBuffer(float* audioBuffer,
                                                    int size,
                                                    void* beepingObject);

/**
 * @brief Retrieve the last decoded string.
 *
 * @param[out] stringDecoded Buffer for the decoded characters (caller
 *        provides — recommended size 30).
 * @param beepingObject Handle from BEEPING_Create().
 * @return 0 if no data available, positive with length on valid data,
 *         negative with length magnitude if the data failed integrity
 *         checks.
 */
BEEPING_DLLEXPORT int32_t BEEPING_GetDecodedData(char* stringDecoded,
                                                 void* beepingObject);

///@}

/** @name Reception quality metrics */
///@{

/**
 * @brief Combined reception-quality score (0.0 poor — 1.0 ideal).
 */
BEEPING_DLLEXPORT float BEEPING_GetConfidence(void* beepingObject);

/**
 * @brief Confidence derived from Reed-Solomon corrections (0.0 — 1.0).
 */
BEEPING_DLLEXPORT float BEEPING_GetConfidenceError(void* beepingObject);

/**
 * @brief Confidence derived from signal-to-noise ratio (0.0 — 1.0).
 */
BEEPING_DLLEXPORT float BEEPING_GetConfidenceNoise(void* beepingObject);

/**
 * @brief Average received beep volume in dB for the last transmission.
 */
BEEPING_DLLEXPORT float BEEPING_GetReceivedBeepsVolume(void* beepingObject);

/**
 * @brief Mode that was decoded when BEEPING_MODE_ALL is active.
 * @return 0 = AUDIBLE, 1 = INAUDIBLE.
 */
BEEPING_DLLEXPORT int32_t BEEPING_GetDecodedMode(void* beepingObject);

///@}

/** @name Decoding frequency range */
///@{

/**
 * @brief Lower bound of the decoding frequency range (Hz).
 */
BEEPING_DLLEXPORT float BEEPING_GetDecodingBeginFreq(void* beepingObject);

/**
 * @brief Upper bound of the decoding frequency range (Hz).
 */
BEEPING_DLLEXPORT float BEEPING_GetDecodingEndFreq(void* beepingObject);

///@}

#ifdef __cplusplus
}
#endif  //__cplusplus

#endif  //__BEEPINGCORELIB_API__
