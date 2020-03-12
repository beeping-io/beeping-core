#ifndef __ENCODER__
#define __ENCODER__

namespace BEEPING
{
  class ReedSolomon;

  class Encoder
  {
  public:
    Encoder(float samplingRate, int buffsize, int windowSize, int numTokens, int numTones);
    ~Encoder(void);

    int SetAudioSignature(int samplesSize, const float *samplesBuffer);

    ReedSolomon *mReedSolomon;
    virtual int EncodeDataToAudioBuffer(const char *stringToEncode, int type, int size, const char *melodyString, int melodySize);
    int GetEncodedAudioBuffer(float *audioBuffer);
    int ResetEncodedAudioBuffer();
        
    //float fastSin(float x);

    int mnAudioSignatureSamples;
    float *mAudioSignature;

    float *mAudioBufferEncodedString;

    int mNumTokens;
    int mNumTones;

    float mSampleRate;
    int mReadIndexEncodedAudioBuffer;
    int mNumMaxSamplesEncodedString;
    int mNumSamplesEncodedString;
    int mBufferSize;

    int mWindowSize;

  };
}

#endif //__ENCODER__