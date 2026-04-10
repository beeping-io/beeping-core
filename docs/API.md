# BeepingCoreLib API Reference

**Version:** 1.0.0 (20220426)

BeepingCoreLib is a C/C++ library for transmitting and receiving numeric data through sound. It exposes a flat C API (`extern "C"`) suitable for FFI bindings from any language, including Flutter (via `dart:ffi`), Swift, and Kotlin/JNI.

---

## Table of Contents

- [Operating Modes](#operating-modes)
- [Public C API (BeepingCoreLib\_api.h)](#public-c-api-beepingcorelib_apih)
  - [Lifecycle](#lifecycle)
  - [Versioning](#versioning)
  - [Configuration](#configuration)
  - [Encoding](#encoding)
  - [Decoding](#decoding)
  - [Query and Status](#query-and-status)
  - [Custom Mode](#custom-mode)
  - [Synth Mode](#synth-mode)
  - [Diagnostics](#diagnostics)
- [Typical Usage Flow](#typical-usage-flow)
- [Return Code Conventions](#return-code-conventions)
- [Internal Components](#internal-components)
  - [Encoder (Encoder.h)](#encoder-encoderh)
  - [Decoder (Decoder.h)](#decoder-decoderh)
  - [SpectralAnalysis (SpectralAnalysis.h)](#spectralanalysis-spectralanalysish)
  - [ReedSolomon (ReedSolomon.h)](#reedsolomon-reedsolomonh)
  - [Globals (Globals.h)](#globals-globalsh)
  - [Encoder Variants](#encoder-variants)
  - [Decoder Variants](#decoder-variants)
  - [FFT (fftsg.h)](#fft-fftsgh)

---

## Operating Modes

The library supports several encoding/decoding modes, selected at configuration time:

| Enum Value | Integer | Description | Status |
|---|---|---|---|
| `BEEPING_MODE_AUDIBLEOLD` | 0 | Legacy audible mode | **Deprecated** |
| `BEEPING_MODE_NONAUDIBLEOLD` | 1 | Legacy non-audible mode | **Deprecated** |
| `BEEPING_MODE_AUDIBLE` | 2 | Audible multi-tone encoding | Active |
| `BEEPING_MODE_NONAUDIBLE` | 3 | Non-audible (ultrasonic) multi-tone encoding | Active |
| `BEEPING_MODE_HIDDEN` | 4 | Hidden multi-tone encoding | Active |
| `BEEPING_MODE_ALL` | 5 | Decode all modes simultaneously | Active |
| `BEEPING_MODE_CUSTOM` | 6 | User-defined base frequency and tone separation | Active |

Only modes 2 through 6 should be used. Modes 0 and 1 are retained for backward compatibility and must not be used in new integrations.

---

## Public C API (BeepingCoreLib\_api.h)

All functions use a flat C calling convention and are exported with `BEEPING_DLLEXPORT`. The library is stateful: callers first create an opaque object with `BEEPING_Create()`, then pass it to every subsequent call.

### Lifecycle

These functions manage the library instance. Every wrapper **must** call these.

#### `BEEPING_Create`

Creates a new BeepingCore instance.

| | |
|---|---|
| **Signature** | `void* BEEPING_Create()` |
| **Parameters** | None |
| **Returns** | Opaque pointer to the internal BeepingCore object. Pass this to all other API functions. |
| **Wrapper relevance** | **Required** |

#### `BEEPING_Destroy`

Destroys a BeepingCore instance and frees all associated memory.

| | |
|---|---|
| **Signature** | `void BEEPING_Destroy(void* beepingObject)` |
| **Returns** | `void` |
| **Wrapper relevance** | **Required** |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

### Versioning

#### `BEEPING_GetVersion`

Returns a pointer to a static string containing version information.

| | |
|---|---|
| **Signature** | `const char* BEEPING_GetVersion()` |
| **Parameters** | None |
| **Returns** | Null-terminated version string (static lifetime, do not free) |
| **Wrapper relevance** | Optional -- useful for diagnostics |

#### `BEEPING_GetVersionInfo`

Writes version information into a caller-provided buffer.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_GetVersionInfo(char* versioninfo)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | Optional -- useful for diagnostics |

| Parameter | Type | Description |
|---|---|---|
| `versioninfo` | `char*` | Caller-allocated buffer to receive the version string |

---

### Configuration

#### `BEEPING_Configure`

Configures the library for a specific operating mode and audio format. Must be called once after `BEEPING_Create()` and before any encode/decode operations.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_Configure(int mode, float samplingRate, int32_t bufferSize, void* beepingObject)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | **Required** |

| Parameter | Type | Description |
|---|---|---|
| `mode` | `int` | Operating mode (use `BEEPING_MODE_AUDIBLE` = 2, `BEEPING_MODE_NONAUDIBLE` = 3, `BEEPING_MODE_HIDDEN` = 4, `BEEPING_MODE_ALL` = 5, or `BEEPING_MODE_CUSTOM` = 6) |
| `samplingRate` | `float` | Audio sampling rate in Hz (typically `44100.0`) |
| `bufferSize` | `int32_t` | Size of audio buffers that will be passed to encode/decode functions |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_SetAudioSignature`

Sets a custom audio signature (short sound clip) that will be mixed on top of the encoded beeps during playback. This allows branding the transmission with a recognizable sound.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_SetAudioSignature(int32_t samplesSize, const float* samplesBuffer, void* beepingObject)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | Optional -- for branded audio experiences |

| Parameter | Type | Description |
|---|---|---|
| `samplesSize` | `int32_t` | Number of samples in `samplesBuffer` (max 2 seconds at 44.1 kHz = 88200 samples) |
| `samplesBuffer` | `const float*` | Array of audio samples (44 kHz, mono, float normalized) |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

### Encoding

These functions encode a character string into an audio signal.

#### `BEEPING_EncodeDataToAudioBuffer`

Encodes a string of characters into an internal audio buffer. After calling this, retrieve the audio data in chunks using `BEEPING_GetEncodedAudioBuffer`.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_EncodeDataToAudioBuffer(const char* stringToEncode, int32_t size, int32_t type, const char* melodyString, int32_t melodySize, void* beepingObject)` |
| **Returns** | Total number of samples in the encoded audio buffer |
| **Wrapper relevance** | **Required** for sending |

| Parameter | Type | Description |
|---|---|---|
| `stringToEncode` | `const char*` | String containing the characters to encode |
| `size` | `int32_t` | Number of characters in `stringToEncode` |
| `type` | `int32_t` | Encoding style: `0` = tones only, `1` = tones + R2D2-style sounds, `2` = tones + melody |
| `melodyString` | `const char*` | String containing note characters for melody synthesis (pass `NULL` if `type` is 0 or 1) |
| `melodySize` | `int32_t` | Number of notes in `melodyString` (pass `0` if `type` is 0 or 1) |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_GetEncodedAudioBuffer`

Reads the next chunk of encoded audio data from the internal buffer. Call repeatedly until the return value is `0` or less than `bufferSize` (indicating the end of the encoded data has been reached).

| | |
|---|---|
| **Signature** | `int32_t BEEPING_GetEncodedAudioBuffer(float* audioBuffer, void* beepingObject)` |
| **Returns** | Number of samples written to `audioBuffer`. Returns `0` or a value less than `bufferSize` when the end of data is reached. |
| **Wrapper relevance** | **Required** for sending |

| Parameter | Type | Description |
|---|---|---|
| `audioBuffer` | `float*` | Caller-allocated buffer of size `bufferSize` (as configured) to receive audio samples |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_ResetEncodedAudioBuffer`

Resets the internal read index so that `BEEPING_GetEncodedAudioBuffer` will start reading from the beginning again. Useful for replaying the same encoded data without re-encoding.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_ResetEncodedAudioBuffer(void* beepingObject)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | Optional -- for replay scenarios |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

### Decoding

These functions decode audio data captured from a microphone back into character data.

#### `BEEPING_DecodeAudioBuffer`

Feeds a buffer of audio samples into the decoder. Call this repeatedly with successive chunks of captured audio.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_DecodeAudioBuffer(float* audioBuffer, int size, void* beepingObject)` |
| **Returns** | Status code (see table below) |
| **Wrapper relevance** | **Required** for receiving |

| Parameter | Type | Description |
|---|---|---|
| `audioBuffer` | `float*` | Array of audio samples to decode |
| `size` | `int` | Number of samples in `audioBuffer` |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

**Return values:**

| Value | Meaning |
|---|---|
| `-1` | No decoded data found (continue feeding audio) |
| `-2` | Start token detected (transmission beginning) |
| `-3` | Complete word decoded (call `BEEPING_GetDecodedData` to retrieve it) |
| `>= 0` | Individual character decoded; the value is the token index |

#### `BEEPING_GetDecodedData`

Retrieves the last fully decoded string after `BEEPING_DecodeAudioBuffer` returns `-3`.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_GetDecodedData(char* stringDecoded, void* beepingObject)` |
| **Returns** | `0` if no data available, `>0` if data is valid (value = number of characters), `<0` if data is available but contains errors (absolute value = number of characters) |
| **Wrapper relevance** | **Required** for receiving |

| Parameter | Type | Description |
|---|---|---|
| `stringDecoded` | `char*` | Caller-allocated buffer to receive the decoded string (allocate at least 30 bytes, per `MAX_DECODE_STRING_SIZE`) |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

### Query and Status

#### `BEEPING_GetConfidence`

Returns an overall reception quality measure combining noise and error correction confidence.

| | |
|---|---|
| **Signature** | `float BEEPING_GetConfidence(void* beepingObject)` |
| **Returns** | `float` in range `[0.0, 1.0]` where `1.0` = ideal reception |
| **Wrapper relevance** | **Recommended** -- display to user as signal quality indicator |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_GetConfidenceError`

Returns the confidence component derived from the number of tokens corrected by the Reed-Solomon error correction algorithm. Lower values indicate more corrections were needed.

| | |
|---|---|
| **Signature** | `float BEEPING_GetConfidenceError(void* beepingObject)` |
| **Returns** | `float` in range `[0.0, 1.0]` |
| **Wrapper relevance** | Optional -- for advanced diagnostics |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_GetConfidenceNoise`

Returns the confidence component derived from the signal-to-noise ratio of received beeps.

| | |
|---|---|
| **Signature** | `float BEEPING_GetConfidenceNoise(void* beepingObject)` |
| **Returns** | `float` in range `[0.0, 1.0]` |
| **Wrapper relevance** | Optional -- for advanced diagnostics |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_GetReceivedBeepsVolume`

Returns the average volume of the last received beep transmission, measured in decibels.

| | |
|---|---|
| **Signature** | `float BEEPING_GetReceivedBeepsVolume(void* beepingObject)` |
| **Returns** | Volume in dB |
| **Wrapper relevance** | Optional -- useful for "move closer" UI prompts |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_GetDecodedMode`

When the decoder is configured in `BEEPING_MODE_ALL`, this function reports which mode the detected transmission was actually using. For single-mode configurations, this always returns the configured mode.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_GetDecodedMode(void* beepingObject)` |
| **Returns** | `0` = Audible, `1` = Non-Audible, `2` = Hidden |
| **Wrapper relevance** | Recommended when using `BEEPING_MODE_ALL` |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_GetDecodingBeginFreq`

Returns the lower bound frequency (in Hz) of the active decoding range.

| | |
|---|---|
| **Signature** | `float BEEPING_GetDecodingBeginFreq(void* beepingObject)` |
| **Returns** | Frequency in Hz |
| **Wrapper relevance** | Optional -- useful for spectrum visualization |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_GetDecodingEndFreq`

Returns the upper bound frequency (in Hz) of the active decoding range.

| | |
|---|---|
| **Signature** | `float BEEPING_GetDecodingEndFreq(void* beepingObject)` |
| **Returns** | Frequency in Hz |
| **Wrapper relevance** | Optional -- useful for spectrum visualization |

| Parameter | Type | Description |
|---|---|---|
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

### Custom Mode

These functions are only relevant when `BEEPING_MODE_CUSTOM` (6) is used.

#### `BEEPING_SetCustomBaseFreq`

Sets the base frequency and inter-beep separation for custom mode encoding/decoding.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_SetCustomBaseFreq(float baseFreq, int beepsSeparation, void* beepingObject)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | Required only for custom mode |

| Parameter | Type | Description |
|---|---|---|
| `baseFreq` | `float` | Base frequency in Hz for the lowest token |
| `beepsSeparation` | `int` | Frequency bin separation between consecutive beep tones |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

### Synth Mode

These functions control how the encoded audio is synthesized.

#### `BEEPING_SetSynthMode`

Sets the synthesis mode used when generating encoded audio.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_SetSynthMode(int synthMode, void* beepingObject)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | Optional |

| Parameter | Type | Description |
|---|---|---|
| `synthMode` | `int` | Synthesis mode identifier |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

#### `BEEPING_SetSynthVolume`

Sets the output volume for synthesized audio.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_SetSynthVolume(float synthVolume, void* beepingObject)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | Optional |

| Parameter | Type | Description |
|---|---|---|
| `synthVolume` | `float` | Volume level for synthesis output |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

### Diagnostics

#### `BEEPING_GetSpectrum`

Copies the current frequency spectrum magnitude data into a caller-provided buffer. Primarily intended for debugging and visualization.

| | |
|---|---|
| **Signature** | `int32_t BEEPING_GetSpectrum(float* spectrumBuffer, void* beepingObject)` |
| **Returns** | `0` on success, `<0` on failure |
| **Wrapper relevance** | Optional -- for debug visualizations only |

| Parameter | Type | Description |
|---|---|---|
| `spectrumBuffer` | `float*` | Caller-allocated buffer to receive spectrum magnitude data |
| `beepingObject` | `void*` | Instance created by `BEEPING_Create()` |

---

## Typical Usage Flow

### Sending (Encoding)

```
1. obj = BEEPING_Create()
2. BEEPING_Configure(mode, 44100.0, bufferSize, obj)
3. BEEPING_EncodeDataToAudioBuffer("12345", 5, 0, NULL, 0, obj)
4. Loop:
     n = BEEPING_GetEncodedAudioBuffer(buffer, obj)
     -> play buffer through speaker
     -> break when n == 0 or n < bufferSize
5. BEEPING_Destroy(obj)
```

### Receiving (Decoding)

```
1. obj = BEEPING_Create()
2. BEEPING_Configure(mode, 44100.0, bufferSize, obj)
3. Loop (feed microphone audio):
     result = BEEPING_DecodeAudioBuffer(micBuffer, size, obj)
     if result == -2 -> start token detected
     if result == -3 -> call BEEPING_GetDecodedData(decoded, obj)
                        confidence = BEEPING_GetConfidence(obj)
                        -> done
4. BEEPING_Destroy(obj)
```

---

## Return Code Conventions

Unless otherwise noted, all functions returning `int32_t` follow these conventions:

| Value | Meaning |
|---|---|
| `0` | Success |
| `< 0` | Error |
| `> 0` | Context-dependent (sample count, character count, etc.) |

`BEEPING_DecodeAudioBuffer` uses a specialized set of return codes documented in its own section above.

---

## Internal Components

The following classes form the internal architecture of BeepingCoreLib. They are **not** exposed through the C API and are **not needed by wrapper libraries**. They are documented here for contributors and maintainers.

### Encoder (Encoder.h)

**Namespace:** `BEEPING`

Base class for all encoding variants. Manages the audio buffer where encoded data is written and provides the core encode/read/reset cycle.

| Method | Signature | Description |
|---|---|---|
| Constructor | `Encoder(float samplingRate, int buffsize, int windowSize, int numTokens, int numTones)` | Initializes encoder with audio parameters and token/tone counts |
| Destructor | `~Encoder()` | Frees all allocated buffers |
| `SetAudioSignature` | `int SetAudioSignature(int samplesSize, const float* samplesBuffer)` | Stores a custom audio signature to overlay on encoded output |
| `EncodeDataToAudioBuffer` | `virtual int EncodeDataToAudioBuffer(const char* stringToEncode, int type, int size, const char* melodyString, int melodySize)` | Encodes a character string into the internal audio buffer. Virtual -- overridden by each mode variant. |
| `GetEncodedAudioBuffer` | `int GetEncodedAudioBuffer(float* audioBuffer)` | Reads the next chunk of encoded audio into the provided buffer |
| `ResetEncodedAudioBuffer` | `int ResetEncodedAudioBuffer()` | Resets the read pointer to replay the encoded audio |

**Key members:** `mReedSolomon` (Reed-Solomon encoder instance), `mAudioBufferEncodedString` (the encoded PCM buffer), `mSampleRate`, `mBufferSize`, `mWindowSize`.

---

### Decoder (Decoder.h)

**Namespace:** `BEEPING`

Base class for all decoding variants. Implements circular buffering, spectral analysis, token detection, and Reed-Solomon error correction.

**Constants:**
- `MAX_DECODE_STRING_SIZE` = 30 (maximum decoded string length)

**Internal struct `sTokenProbs`:** Holds per-token probability statistics used during decoding decisions (token index, energy ratios, tone indices, energy values).

| Method | Signature | Description |
|---|---|---|
| Constructor | `Decoder(float sr, int buffsize, int windowSize, int numTokens, int numTones)` | Initializes decoder with audio parameters |
| Destructor | `~Decoder()` | Frees all allocated buffers |
| `DecodeAudioBuffer` | `virtual int DecodeAudioBuffer(float* audioBuffer, int size)` | Processes audio samples and returns decode status |
| `GetDecodedData` | `virtual int GetDecodedData(char* stringDecoded)` | Retrieves the decoded string |
| `GetConfidence` | `float GetConfidence()` | Returns combined confidence score [0.0, 1.0] |
| `GetConfidenceError` | `float GetConfidenceError()` | Returns error-correction confidence component |
| `GetConfidenceNoise` | `float GetConfidenceNoise()` | Returns signal-to-noise confidence component |
| `GetReceivedBeepsVolume` | `float GetReceivedBeepsVolume()` | Returns average received volume in dB |
| `GetDecodedMode` | `int GetDecodedMode()` | Returns the detected decoding mode (for multi-mode decoding) |
| `GetDecodingBeginFreq` | `virtual float GetDecodingBeginFreq()` | Returns lower bound of decoding frequency range |
| `GetDecodingEndFreq` | `virtual float GetDecodingEndFreq()` | Returns upper bound of decoding frequency range |
| `GetSpectrum` | `int GetSpectrum(float* spectrumBuffer)` | Copies current spectrum data to buffer |
| `AnalyzeStartTokens` | `virtual int AnalyzeStartTokens(float* audioBuffer)` | Analyzes audio for the presence of start (front-door) tokens |
| `AnalyzeToken` | `virtual int AnalyzeToken(float* audioBuffer)` | Analyzes audio for individual data tokens |
| `ComputeStatsStartTokens` | `virtual int ComputeStatsStartTokens()` | Computes statistical measures for start token detection |
| `ComputeStats` | `virtual int ComputeStats()` | Computes statistical measures for data token detection |
| `DeReverbToken` | `virtual int DeReverbToken(const int nbins, int* freqsBins)` | Compensates for reverberation artifacts in token detection |
| `ComputeBlockMagSpecSumsCurrentToken` | `virtual float ComputeBlockMagSpecSumsCurrentToken(int midFreqBin, int width, int nbins, vector<float>& sumPerFrame)` | Computes magnitude spectrum sums for the current token's time block |
| `ComputeBlockMagSpecSumsLastToken` | `virtual float ComputeBlockMagSpecSumsLastToken(int midFreqBin, int width, int nbins, vector<float>& sumPerFrame)` | Computes magnitude spectrum sums for the previous token's time block |

---

### SpectralAnalysis (SpectralAnalysis.h)

**Namespace:** `BEEPING`

Performs windowed FFT-based spectral analysis on audio frames. Used internally by all Decoder variants.

**Enum `Mode`:**

| Value | Meaning |
|---|---|
| `kMagnitudeSpectrum` (0) | Compute magnitude spectrum |
| `kEnergySpectrum` (1) | Compute energy (power) spectrum |

| Method | Signature | Description |
|---|---|---|
| Constructor | `SpectralAnalysis(Mode mode, int fftSize, int windowSize, int hopSize)` | Initializes with analysis mode and FFT parameters |
| Destructor | `~SpectralAnalysis()` | Frees FFT and buffer resources |
| `doFFT` | `void doFFT(float* inputBuffer, float* magSpectrum, float* imagSpectrum)` | Performs FFT on `inputBuffer`, outputs magnitude and imaginary spectra |
| `multiplyBuffers` | `int multiplyBuffers(float* tgt, const float* srcA, const float* srcB, int n)` | Element-wise buffer multiplication (used for windowing) |
| `clearBuffer` | `void clearBuffer(float* tgt, int n)` | Zeros out a buffer |
| `copyToBuffer` | `int copyToBuffer(float* tgt, const float* src, int n)` | Copies `n` samples between buffers |
| `generateBlackmanHarris92Window` | `void generateBlackmanHarris92Window(float* window, int size)` | Generates a 92 dB Blackman-Harris window |
| `generateBlackmanHarris74Window` | `void generateBlackmanHarris74Window(float* window, int size)` | Generates a 74 dB Blackman-Harris window |
| `generateBlackmanHarrisWindow` | `void generateBlackmanHarrisWindow(float* window, int size)` | Generates a standard Blackman-Harris window |

**Key members:** `mFftSize`, `mWindowSize`, `mHopSize`, `mSpecSize`, `mSpecMag` (magnitude spectrum buffer), `mSpecPhase` (phase spectrum buffer), `mWindow` (analysis window).

---

### ReedSolomon (ReedSolomon.h)

**Namespace:** `BEEPING`

Implements Reed-Solomon error correction coding over GF(2^4). Provides forward error correction so that transmissions can survive partial data loss or noise corruption.

| Method | Signature | Description |
|---|---|---|
| Constructor | `ReedSolomon()` | Initializes RS parameters |
| Destructor | `~ReedSolomon()` | Frees polynomial and field tables |
| `GenerateGaloisField` | `void GenerateGaloisField()` | Precomputes the Galois field lookup tables (`alpha_to`, `index_of`) |
| `GeneratePoly` | `void GeneratePoly()` | Generates the generator polynomial for encoding |
| `Encode` | `void Encode()` | Encodes the data in `data[]` into parity symbols in `bb[]` |
| `Decode` | `void Decode()` | Decodes and error-corrects the received codeword in `recd[]` |
| `SetMessage` | `void SetMessage(const vector<int> message)` | Sets the message symbols to encode (high-level API) |
| `GetCode` | `void GetCode(vector<int>& code)` | Retrieves the encoded codeword including parity (high-level API) |
| `SetCode` | `void SetCode(const vector<int> code)` | Sets the received codeword for decoding (high-level API) |
| `GetMessage` | `void GetMessage(vector<int>& message)` | Retrieves the decoded and corrected message symbols (high-level API) |

**Key parameters:** `mm` (field order, GF(2^mm)), `nn` (codeword length = 2^mm - 1), `tt` (correctable errors), `kk` (message length = nn - 2*tt), `msg_len` (actual message length for shortened codes).

---

### Globals (Globals.h)

**Namespace:** `Globals`

A collection of global constants, lookup tables, and utility functions shared across all encoder and decoder variants.

#### Initialization

| Function | Signature | Description |
|---|---|---|
| `init` | `int init(int fftsize, float samplerate)` | Initializes global lookup tables for the given FFT size and sample rate |

#### Character-Index Mapping

| Function | Signature | Description |
|---|---|---|
| `getIdxFromChar` | `int getIdxFromChar(char c)` | Converts a character to its token index |
| `getCharFromIdx` | `char getCharFromIdx(int idx)` | Converts a token index back to its character |

#### Frequency Mapping (per mode)

Each mode has a set of functions to convert between token indices and frequencies/bins:

| Pattern | Functions Available |
|---|---|
| Single-tone | `getFreqFromIdxAudible`, `getFreqFromIdxNonAudible` |
| Multi-tone frequencies | `getFreqsFromIdx{Audible,NonAudible,Hidden,Custom}MultiTone` |
| Multi-tone primary tone | `getToneFromIdx{Audible,NonAudible,Hidden,Custom}MultiTone` |
| Multi-tone bin indices | `getIdxsFromIdx{Audible,NonAudible,Hidden,Custom}MultiTone` |
| Reverse lookup (tones to token) | `getIdxTokenFromIdxsTones{Audible,NonAudible,Hidden,Custom}MultiTone` |

#### Loudness Mapping

| Function | Description |
|---|---|
| `getLoudnessFromIdx(int idx)` | Returns loudness compensation factor for a single-tone token |
| `getLoudnessAudibleMultiToneFromIdx(int idx, float** freqsLoudness)` | Returns per-frequency loudness for audible multi-tone |
| `getLoudnessNonAudibleMultiToneFromIdx(int idx, float** freqsLoudness)` | Returns per-frequency loudness for non-audible multi-tone |
| `getLoudnessHiddenMultiToneFromIdx(int idx, float** freqsLoudness)` | Returns per-frequency loudness for hidden multi-tone |
| `getLoudnessCustomMultiToneFromIdx(int idx, float** freqsLoudness)` | Returns per-frequency loudness for custom multi-tone |

#### Utility Functions

| Function | Signature | Description |
|---|---|---|
| `getMusicalNoteFromIdx` | `float getMusicalNoteFromIdx(int idx)` | Returns the musical note frequency for a token index |
| `maxValue` | `float maxValue(float* array, int size)` | Returns the maximum value in a float array |
| `maxValue` | `int maxValue(int* array, int size)` | Returns the maximum value in an int array |
| `secondValue` | `float secondValue(float* array, int size)` | Returns the second-largest value in a float array |
| `maxValueIdx` | `int maxValueIdx(float* array, int size)` | Returns the index of the maximum value (float) |
| `maxValueIdx` | `int maxValueIdx(int* array, int size)` | Returns the index of the maximum value (int) |
| `secondValueIdx` | `int secondValueIdx(float* array, int size)` | Returns the index of the second-largest value (float) |
| `secondValueIdx` | `int secondValueIdx(int* array, int size)` | Returns the index of the second-largest value (int) |
| `sum` | `float sum(float* data, int size)` | Sum of all elements |
| `square_sum` | `float square_sum(float* data, int size)` | Sum of squared elements |
| `mean` | `float mean(float* data, int size)` | Arithmetic mean |
| `standard_deviation` | `float standard_deviation(float* data, int size)` | Standard deviation (computes mean internally) |
| `standard_deviation` | `float standard_deviation(float* data, float mean, int size)` | Standard deviation with precomputed mean |

#### Global Constants

| Constant | Type | Description |
|---|---|---|
| `durToken` | `float` | Duration of a single token in seconds |
| `durFade` | `float` | Duration of fade-in/fade-out between tokens |
| `pi` | `float` | Pi constant |
| `two_pi` | `float` | 2 * Pi constant |
| `tokenAmplitude` | `float` | Default amplitude for generated tones |
| `numTokensAll` | `int` | Total token count across all modes |
| `numTokensAudible` | `int` | Token count for audible mode |
| `numTokensNonAudible` | `int` | Token count for non-audible mode |
| `numTokensHidden` | `int` | Token count for hidden mode |
| `numTokensCustom` | `int` | Token count for custom mode |
| `numTones*` | `int` | Tone counts per multi-tone mode variant |
| `nBinsOffsetFor*MultiTone` | `int` | FFT bin offsets per mode |
| `freqOffsetFor*MultiTone` | `float` | Frequency offsets per mode |
| `freqBaseForCustomMultiTone` | `float` | User-set base frequency for custom mode |
| `beepsSeparationForCustomMultiTone` | `int` | User-set bin separation for custom mode |
| `synthMode` | `int` | Current synthesis mode |
| `synthVolume` | `float` | Current synthesis volume |
| `frontDoorTokens` | `char[2]` | The two front-door (start) token characters |
| `numFrontDoorTokens` | `const int` | Number of front-door tokens (2) |
| `numWordTokens` | `const int` | Number of payload data tokens |
| `numCorrectionTokens` | `const int` | Number of Reed-Solomon parity tokens |
| `numCheckTokens` | `const int` | Number of check tokens |
| `numMessageTokens` | `const int` | Number of message tokens |
| `numTotalTokens` | `const int` | Total tokens per transmission |

---

### Encoder Variants

All encoder variants inherit from `Encoder` and override `EncodeDataToAudioBuffer` to generate mode-specific tone patterns. The public interface is identical across all variants.

| Class | Header | Mode | Description |
|---|---|---|---|
| `EncoderAudible` | `EncoderAudible.h` | Legacy audible (single-tone) | Deprecated single-tone audible encoder |
| `EncoderNonAudible` | `EncoderNonAudible.h` | Legacy non-audible (single-tone) | Deprecated single-tone ultrasonic encoder |
| `EncoderAudibleMultiTone` | `EncoderAudibleMultiTone.h` | Audible multi-tone | Active audible encoder using multiple simultaneous tones per token |
| `EncoderNonAudibleMultiTone` | `EncoderNonAudibleMultiTone.h` | Non-audible multi-tone | Active ultrasonic encoder using multiple simultaneous tones |
| `EncoderHiddenMultiTone` | `EncoderHiddenMultiTone.h` | Hidden multi-tone | Active hidden-frequency encoder using multiple simultaneous tones |
| `EncoderCustomMultiTone` | `EncoderCustomMultiTone.h` | Custom multi-tone | Encoder with user-defined base frequency and separation |

Multi-tone variants add `mCurrentFreqs` and `mCurrentFreqsLoudness` arrays for per-frame frequency and loudness tracking.

---

### Decoder Variants

All decoder variants inherit from `Decoder` and override the virtual analysis and statistics methods. The public interface is identical across all variants.

| Class | Header | Mode | Description |
|---|---|---|---|
| `DecoderAudible` | `DecoderAudible.h` | Legacy audible (single-tone) | Deprecated single-tone audible decoder |
| `DecoderNonAudible` | `DecoderNonAudible.h` | Legacy non-audible (single-tone) | Deprecated single-tone ultrasonic decoder |
| `DecoderAudibleMultiTone` | `DecoderAudibleMultiTone.h` | Audible multi-tone | Active audible decoder |
| `DecoderNonAudibleMultiTone` | `DecoderNonAudibleMultiTone.h` | Non-audible multi-tone | Active ultrasonic decoder |
| `DecoderHiddenMultiTone` | `DecoderHiddenMultiTone.h` | Hidden multi-tone | Active hidden-frequency decoder |
| `DecoderCustomMultiTone` | `DecoderCustomMultiTone.h` | Custom multi-tone | Decoder with user-defined frequency parameters; also overrides `GetDecodingBeginFreq`/`GetDecodingEndFreq` |
| `DecoderAllMultiTone` | `DecoderAllMultiTone.h` | All modes simultaneously | Runs audible, non-audible, and hidden decoders in parallel; reports which mode was detected |

Single-tone decoder variants (`DecoderAudible`, `DecoderNonAudible`) add neighbor-bin analysis members (`mSizeNighbBins`, `mEvalNeighbTokenMags`). Multi-tone variants add tone-tracking arrays (`mIdxs`, `mBlockEnergyRatiosMaxToneIdx`, `mToneRepetitions`, `idxTonesFrontDoorToken*`). The `DecoderAllMultiTone` variant additionally maintains arrays of arrays (`*Array`) for parallel multi-mode tracking.

---

### FFT (fftsg.h)

Wraps the Ooura FFT library (`CFFTOoura` class) providing split-radix FFT for power-of-2 data lengths. Functions include `cdft` (complex DFT), `rdft` (real DFT), `ddct` (DCT), `ddst` (DST), `dfct`, and `dfst`. Used internally by `SpectralAnalysis`. Not relevant for wrapper development.

Supporting types and macros are defined in `defines_ooura.h` (`DATA` = `float`, `INT32` = `int`, `_TWO_PI`, `_PI`, etc.) and `ios_log.h` (platform-specific logging for iOS).

---

## Summary: Functions Relevant for Upper-Layer Wrappers

The following table lists every C API function and its relevance to Flutter, Swift, or Kotlin wrapper libraries that integrate BeepingCoreLib via FFI.

| Function | Category | Wrapper Must Call |
|---|---|---|
| `BEEPING_Create` | Lifecycle | **Yes** |
| `BEEPING_Destroy` | Lifecycle | **Yes** |
| `BEEPING_Configure` | Configuration | **Yes** |
| `BEEPING_EncodeDataToAudioBuffer` | Encoding | **Yes** (for send) |
| `BEEPING_GetEncodedAudioBuffer` | Encoding | **Yes** (for send) |
| `BEEPING_DecodeAudioBuffer` | Decoding | **Yes** (for receive) |
| `BEEPING_GetDecodedData` | Decoding | **Yes** (for receive) |
| `BEEPING_GetConfidence` | Status | **Recommended** |
| `BEEPING_GetDecodedMode` | Status | **Recommended** (when using ALL mode) |
| `BEEPING_ResetEncodedAudioBuffer` | Encoding | Optional |
| `BEEPING_SetAudioSignature` | Configuration | Optional |
| `BEEPING_SetCustomBaseFreq` | Configuration | Only for custom mode |
| `BEEPING_SetSynthMode` | Configuration | Optional |
| `BEEPING_SetSynthVolume` | Configuration | Optional |
| `BEEPING_GetVersion` | Versioning | Optional |
| `BEEPING_GetVersionInfo` | Versioning | Optional |
| `BEEPING_GetConfidenceError` | Status | Optional |
| `BEEPING_GetConfidenceNoise` | Status | Optional |
| `BEEPING_GetReceivedBeepsVolume` | Status | Optional |
| `BEEPING_GetDecodingBeginFreq` | Status | Optional |
| `BEEPING_GetDecodingEndFreq` | Status | Optional |
| `BEEPING_GetSpectrum` | Diagnostics | Optional (debug only) |

A minimal wrapper needs only the 7 functions marked **Yes**. Adding `BEEPING_GetConfidence` and `BEEPING_GetDecodedMode` is strongly recommended for a complete user experience.
