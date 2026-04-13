#ifdef _ANDROID_LOG_
#include <android/log.h>
#endif  //_ANDROID_LOG_

#ifdef _IOS_LOG_
void ios_log(const char* message, ...) __attribute__((format(printf, 1, 2)));
#endif  //_IOS_LOG_

namespace Globals {

enum DECODING_MODE {
  DECODING_MODE_AUDIBLE = 0,
  DECODING_MODE_NONAUDIBLE = 1,
  DECODING_MODE_HIDDEN = 2,
  DECODING_MODE_CUSTOM = 3
};

// --- Constexpr token/tone counts (immutable across all instances) ---
constexpr int numTokensAll = 32;
constexpr int numTonesAll = 9;

constexpr int numTokensAudible = numTokensAll;
constexpr int numTokensNonAudible = numTokensAll;
constexpr int numTokensHidden = numTokensAll;
constexpr int numTokensCustom = numTokensAll;

constexpr int numTonesAudibleMultiTone = numTonesAll;
constexpr int numTonesNonAudibleMultiTone = numTonesAll;
constexpr int numTonesHiddenMultiTone = numTonesAll;
constexpr int numTonesCustomMultiTone = numTonesAll;

constexpr int numFrontDoorTokens = 2;
constexpr int numWordTokens = 9;
constexpr int numCheckTokens = 1;
constexpr int numCorrectionTokens = 8;
constexpr int numMessageTokens =
    numWordTokens + numCheckTokens + numCorrectionTokens;
constexpr int numTotalTokens = numFrontDoorTokens + numMessageTokens;

// --- Character / frequency lookup (pure functions, no global state) ---
extern int getIdxFromChar(char c);
extern char getCharFromIdx(int idx);

extern float getFreqFromIdxAudible(int idx, float mSampleRate, int mWindowSize);
extern float getFreqFromIdxNonAudible(int idx, float mSampleRate,
                                      int mWindowSize);

extern void getFreqsFromIdxAudibleMultiTone(int idx, float samplingRate,
                                            int windowSize, float** freqs);
extern float getToneFromIdxAudibleMultiTone(int idx, float samplingRate,
                                            int windowSize);
extern void getIdxsFromIdxAudibleMultiTone(int idx, int** idxs);

extern void getFreqsFromIdxNonAudibleMultiTone(int idx, float samplingRate,
                                               int windowSize, float freqOffset,
                                               float** freqs);
extern float getToneFromIdxNonAudibleMultiTone(int idx, float samplingRate,
                                               int windowSize,
                                               float freqOffset);
extern void getIdxsFromIdxNonAudibleMultiTone(int idx, int** idxs);

extern void getFreqsFromIdxHiddenMultiTone(int idx, float samplingRate,
                                           int windowSize, float freqOffset,
                                           float** freqs);
extern float getToneFromIdxHiddenMultiTone(int idx, float samplingRate,
                                           int windowSize, float freqOffset);
extern void getIdxsFromIdxHiddenMultiTone(int idx, int** idxs);

extern void getFreqsFromIdxCustomMultiTone(int idx, float samplingRate,
                                           int windowSize, float freqBase,
                                           float freqOffset, float** freqs);
extern float getToneFromIdxCustomMultiTone(int idx, float samplingRate,
                                           int windowSize, float freqBase,
                                           float freqOffset);
extern void getIdxsFromIdxCustomMultiTone(int idx, int** idxs);

extern int getIdxTokenFromIdxsTonesAudibleMultiTone(int idx1, int idx2);
extern int getIdxTokenFromIdxsTonesNonAudibleMultiTone(int idx1, int idx2);
extern int getIdxTokenFromIdxsTonesHiddenMultiTone(int idx1, int idx2);
extern int getIdxTokenFromIdxsTonesCustomMultiTone(int idx1, int idx2);

extern float getLoudnessFromIdx(int idx, int numTokens);
extern void getLoudnessAudibleMultiToneFromIdx(int idx, float** freqsLoudness);
extern void getLoudnessNonAudibleMultiToneFromIdx(int idx,
                                                  float** freqsLoudness);
extern void getLoudnessHiddenMultiToneFromIdx(int idx, float** freqsLoudness);
extern void getLoudnessCustomMultiToneFromIdx(int idx, float** freqsLoudness);

extern float getMusicalNoteFromIdx(int idx);

extern float maxValue(float* myArray, int size);
extern int maxValue(int* myArray, int size);

extern float secondValue(float* myArray, int size);

extern int maxValueIdx(float* myArray, int size);
extern int maxValueIdx(int* myArray, int size);

extern int secondValueIdx(float* myArray, int size);
extern int secondValueIdx(int* myArray, int size);

extern float sum(float* data, int size);
extern float square_sum(float* data, int size);
extern float mean(float* data, int size);
extern float standard_deviation(float* data, int size);
extern float standard_deviation(float* data, float mean, int size);

}  // namespace Globals
