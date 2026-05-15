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
 * @brief Override the log file path used by the library.
 *
 * Must be called *before* BEEPING_Create() to take effect. The library
 * writes logs to a rotating file at this path; if the path cannot be
 * opened the library falls back to a null sink (no logs) rather than
 * crashing.
 *
 * @param absolutePath Absolute path to the log file. Pass `nullptr` to
 *        reset to the default (`logs/beeping.log` relative to cwd).
 * @return 0 on success; -1 if the logger was already initialized (call
 *         is ignored); -2 if the argument is empty.
 *
 * Notes:
 * - On Android, this is a no-op and returns 0 — the library always writes
 *   to logcat (tag `BeepingCore`), so a writable filesystem is not
 *   required.
 * - On every other platform, if the configured path cannot be opened the
 *   library logs are silently dropped (with a one-time warning to stderr)
 *   instead of crashing the host process.
 */
BEEPING_DLLEXPORT int32_t BEEPING_SetLogPath(const char* absolutePath);

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

/** @name Time-scheduled encoding */
///@{

/**
 * @brief Compute the timestamps (in seconds) at which beeps should fire
 *        within `duration`.
 *
 * Use this for custom mixing loops, UI preview of the schedule, or any
 * wrapper that wants to drive the encoder manually.
 *
 * @param duration       Total duration in seconds. Must be >= 2.3.
 * @param startTime      Offset of the first beep in seconds. Must be >= 0
 *                       and `startTime + 2.3 <= duration`.
 * @param interval       Seconds between successive beeps. Must be > 0.
 * @param[out] outTimestamps Caller-allocated array of `maxTimestamps`
 *                       doubles. May be `nullptr` only when
 *                       `maxTimestamps == 0` (size-query mode).
 * @param maxTimestamps  Capacity of `outTimestamps`. Pass 0 to query the
 *                       count without writing.
 * @param[out] outCount  Number of beeps that fit in the schedule. Pass
 *                       `nullptr` to skip. If `maxTimestamps < outCount`,
 *                       only the first `maxTimestamps` are written and the
 *                       caller can detect the truncation via this value.
 * @return 0 on success; -1 if `outTimestamps` is null with
 *         `maxTimestamps > 0`; -2 on invalid params.
 */
BEEPING_DLLEXPORT int32_t BEEPING_ComputeBeepSchedule(
    float duration, float startTime, float interval,
    double* outTimestamps, int32_t maxTimestamps,
    int32_t* outCount);

/**
 * @brief Required output buffer size (in float samples) for
 *        BEEPING_EncodeWithSchedule at the currently-configured sample rate.
 *
 * @param duration       Total duration in seconds (must be > 0).
 * @param beepingObject  Handle from BEEPING_Create(), already Configure()d.
 * @return `floor(duration * sampleRate)`, or negative on error
 *         (`-1` if `beepingObject` is null, `-2` for invalid params).
 */
BEEPING_DLLEXPORT int32_t BEEPING_GetScheduleBufferSize(
    float duration, void* beepingObject);

/**
 * @brief Encode `code` repeated as multiple beeps scheduled across
 *        `duration` seconds.
 *
 * Each beep's payload is `code` concatenated with the rounded timestamp of
 * the beep in seconds, encoded as 4-char zero-padded base-32 (alphabet
 * `[0-9a-v]`). The decoder side can recover the beep's position within the
 * schedule from this trailing tag.
 *
 * The caller-allocated `outBuffer` is filled directly; silence segments are
 * written as zeros. No internal accumulation — the caller does not need to
 * drain via BEEPING_GetEncodedAudioBuffer afterwards.
 *
 * @param code           Payload (typical: 5-char base-32 [0-9a-v] key).
 * @param codeSize       Length of `code` in bytes.
 * @param type           0 = pure tones, 1 = tones + R2D2, 2 = melody mode.
 * @param melody         Melody payload (only used when `type == 2`).
 * @param melodySize     Length of `melody` (0 for type 0 or 1).
 * @param duration       Total duration in seconds (>= 2.3).
 * @param startTime      Offset of first beep in seconds (>= 0).
 * @param interval       Seconds between beeps (> 0).
 * @param beepGainDb     Beep volume in dB, clamped internally to [-60, +12].
 * @param[out] outBuffer Caller-allocated float array of `maxSamples` floats.
 * @param maxSamples     Capacity of `outBuffer`. Use
 *                       BEEPING_GetScheduleBufferSize() to size it.
 * @param[out] outSamplesWritten On success: actual sample count written
 *                       (`floor(duration * sampleRate)`). On a
 *                       buffer-too-small error: the *required* size, so the
 *                       caller can resize and retry. Pass `nullptr` to skip.
 * @param beepingObject  Handle from BEEPING_Create(), already Configure()d.
 * @return 0 on success; -1 if `outBuffer` is too small (with
 *         `outSamplesWritten` carrying the required size); -2 on invalid
 *         params; -3 if the encoder is not configured.
 */
BEEPING_DLLEXPORT int32_t BEEPING_EncodeWithSchedule(
    const char* code, int32_t codeSize,
    int32_t type, const char* melody, int32_t melodySize,
    float duration, float startTime, float interval,
    float beepGainDb,
    float* outBuffer, int32_t maxSamples,
    int32_t* outSamplesWritten,
    void* beepingObject);

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
 * Safe to call at any point — if no complete word has been decoded yet
 * the function returns 0 and writes a NUL byte at `stringDecoded[0]`.
 * (Older versions could SIGSEGV here; see BEE-2228.) Callers that drive
 * decoding via BEEPING_DecodeAudioBuffer should still typically wait for
 * a `-3` return code (DECODE_COMPLETE) before calling this method.
 *
 * Wire format: when data is available, the decoder writes 9 chars in the
 * `[0-9a-v]` base-32 alphabet — the 5-char user payload followed by a
 * 4-char trailer (timestamp tag for scheduler-encoded streams, or zero
 * padding otherwise). Callers that only want the user payload should
 * truncate to the first 5 chars; callers using
 * BEEPING_EncodeWithSchedule() can split via
 * BEEPING_ParseScheduledPayload() instead.
 *
 * @param[out] stringDecoded Buffer for the decoded characters (caller
 *        provides — recommended size 30).
 * @param beepingObject Handle from BEEPING_Create().
 * @return 0 if no data available, positive with length (typically 9) on
 *         valid data, negative with length magnitude if the data failed
 *         integrity checks.
 */
BEEPING_DLLEXPORT int32_t BEEPING_GetDecodedData(char* stringDecoded,
                                                 void* beepingObject);

/**
 * @brief Split a scheduler-formatted payload into its `code` prefix and
 *        rounded-seconds timestamp suffix.
 *
 * Use this on any payload — including ones obtained outside this library —
 * whose format follows `code + 4-char zero-padded base-32 timestamp`, the
 * layout emitted by BEEPING_EncodeWithSchedule().
 *
 * @param payload         Raw payload bytes.
 * @param payloadSize     Length of `payload`. Must be >= 5 (1 code char
 *                        minimum + 4 timestamp chars).
 * @param[out] outCode    Caller-allocated buffer for the code prefix
 *                        (NUL-terminated). May be `nullptr` if only the
 *                        timestamp is needed.
 * @param maxCodeSize     Capacity of `outCode` including the NUL.
 * @param[out] outCodeSize Length of the code prefix without NUL (==
 *                        `payloadSize - 4`). Pass `nullptr` to skip.
 * @param[out] outTimestampSec Rounded timestamp in seconds. Pass `nullptr`
 *                        to skip.
 * @return 0 on success; -1 if `outCode != nullptr` and `maxCodeSize` is too
 *         small (must hold `payloadSize - 4 + 1` bytes); -2 if the trailing
 *         4 chars are not valid base-32 or the payload is shorter than 5
 *         bytes.
 */
BEEPING_DLLEXPORT int32_t BEEPING_ParseScheduledPayload(
    const char* payload, int32_t payloadSize,
    char* outCode, int32_t maxCodeSize, int32_t* outCodeSize,
    int32_t* outTimestampSec);

/**
 * @brief Convenience wrapper that calls BEEPING_GetDecodedData() and splits
 *        the result via BEEPING_ParseScheduledPayload() in a single step.
 *
 * @param[out] outCode    Caller-allocated buffer for the code prefix
 *                        (NUL-terminated). Recommended size: 30.
 * @param maxCodeSize     Capacity of `outCode` including the NUL.
 * @param[out] outCodeSize Length of the code prefix without NUL.
 *                        Pass `nullptr` to skip.
 * @param[out] outTimestampSec Rounded timestamp in seconds. Pass `nullptr`
 *                        to skip.
 * @param beepingObject   Handle from BEEPING_Create().
 * @return Length of full decoded payload on success (positive); 0 if no
 *         decoded data; negative-with-magnitude on integrity failure
 *         (same convention as BEEPING_GetDecodedData); -10 if the split
 *         step failed (payload < 5 chars or invalid base-32 trailer).
 */
BEEPING_DLLEXPORT int32_t BEEPING_GetDecodedScheduledPayload(
    char* outCode, int32_t maxCodeSize, int32_t* outCodeSize,
    int32_t* outTimestampSec, void* beepingObject);

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
