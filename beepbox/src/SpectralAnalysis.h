#ifndef __SPECTRALANALYSIS__
#define __SPECTRALANALYSIS__

#include "fftsg.h"

namespace BEEPING
{
	enum Mode
	{
		kMagnitudeSpectrum = 0,
		kEnergySpectrum = 1
	};
	
	class SpectralAnalysis
	{
	private:
		SpectralAnalysis(const SpectralAnalysis&);
		void operator = (const SpectralAnalysis&);
		
		Mode mode;
		
    CFFTOoura *fftooura;

		float *inputBuffer;
		
	public:
		SpectralAnalysis(Mode _mode, int _fftSize, int _windowSize, int _hopSize);
		~SpectralAnalysis();

    int mFftSize;
    int mWindowSize;
    int mHopSize;
    int mSpecSize;
    float *mSpecMag;
	  float *mSpecPhase;
    float *mWindow;

    void doFFT(float *inputBuffer, float *magSpectrum, float* imagSpectrum);


    inline int multiplyBuffers(float* tgtPtr, const float* srcPtrA, const float* srcPtrB, int n);
    inline void clearBuffer(float* tgtPtr, int n);
    inline int copyToBuffer(float* tgtPtr, const float* srcPtr, int n);

    inline void generateBlackmanHarris92Window(float* window,int size);
    inline void generateBlackmanHarris74Window(float *window, int size);
    inline void generateBlackmanHarrisWindow(float *window, int size);
	};
	
} // namespace BEEPING

#endif // __SPECTRALANALYSIS__
