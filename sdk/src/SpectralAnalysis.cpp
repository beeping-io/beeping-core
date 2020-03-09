#include "SpectralAnalysis.h"
#include "Globals.h"
#include <cmath>
#include <cstring> // for memset

#include "fftsg.h"

using namespace BEEPING;

SpectralAnalysis::SpectralAnalysis(
               Mode _mode, 
               int _fftSize,
               int _windowSize,
               int _hopSize
               )
:mode(_mode)
{
  mFftSize = _fftSize;
  mWindowSize = _windowSize;
  mHopSize = _hopSize;
  fftooura = new CFFTOoura();
  inputBuffer = new float[mFftSize+1];

  mSpecSize = (mFftSize / 2) +1;
 
  mSpecMag = new float[mSpecSize];
  
	mSpecPhase = new float[mSpecSize];

  mWindow = new float[mWindowSize];
	generateBlackmanHarris74Window(mWindow,mWindowSize);
  //generateBlackmanHarris92Window(mWindow,mWindowSize);

}

SpectralAnalysis::~SpectralAnalysis()
{
  delete fftooura;
  delete [] inputBuffer;

  delete [] mSpecMag;
  
  delete [] mSpecPhase;

  delete [] mWindow;
}


void SpectralAnalysis::doFFT(float *_inputBuffer, float *magSpectrum, float* imagSpectrum)
{
  for (int i=0;i<mFftSize+1;i++)
  {
    inputBuffer[i] = _inputBuffer[i]*mWindow[i];
  }
  
  int magSize = mFftSize / 2;

  fftooura->rdft(magSize*2, 1, inputBuffer);
  for(int i=1; i<magSize; i++)
			inputBuffer[i*2+1] = -inputBuffer[i*2+1];
  
  if (mode == kMagnitudeSpectrum)
  {
    for (int i=1; i<magSize; i++)
    {
      float real = inputBuffer[2*i];
      float imag = inputBuffer[2*i+1];
      magSpectrum[i] = std::sqrt(real * real + imag * imag);
    }
    magSpectrum[0] = std::abs(inputBuffer[0]);
    magSpectrum[magSize] = std::abs(inputBuffer[1]);
  }
  else
  {
    for (int i=1; i<magSize; i++)
    {
      float real = inputBuffer[2*i];
      float imag = inputBuffer[2*i+1];
      magSpectrum[i] = (real * real + imag * imag);
    }
    magSpectrum[0] = inputBuffer[0]*inputBuffer[0];
    magSpectrum[magSize] = inputBuffer[1]*inputBuffer[1];
  }
  
  /*if (realSpectrum)
  {
    for (int i=0; i<=magSize; i++)
    {
      realSpectrum[i] = inputBuffer[2*i];
    }
  }*/
  if (imagSpectrum)
  {
    imagSpectrum[0] = 0.f;
    imagSpectrum[magSize] = 0.f;

    for (int i=0; i<magSize; i++)
    {
      imagSpectrum[i] = inputBuffer[2*i+1];
    }   
  }
}


inline int SpectralAnalysis::multiplyBuffers(float* tgtPtr, const float* srcPtrA, const float* srcPtrB, int n)
{
	float* endPtr = tgtPtr + n;
	
	while (tgtPtr!=endPtr)
	{
		*tgtPtr++ = *srcPtrA++ * *srcPtrB++ ;
	}

	return n;
}


inline void SpectralAnalysis::clearBuffer(float* tgtPtr, int n)
{
	float* endPtr = tgtPtr + n;
	
	while (tgtPtr!=endPtr)
	{
		*tgtPtr++ = 0.f;
	}
}

inline int SpectralAnalysis::copyToBuffer(float* tgtPtr, const float* srcPtr, int n)
{
	float* endPtr = tgtPtr + n;
	
	while (tgtPtr!=endPtr)
	{
		*tgtPtr++ = *srcPtr++;
	}

	return n;
}


inline void SpectralAnalysis::generateBlackmanHarris92Window(float* window,int size)
{
  int i;
	float fSum=0;
	/* for -92dB */
	float a0 = .35875f, a1 = .48829f, a2 = .14128f, a3 = .01168f;
  float fConst = float(Globals::two_pi) / (size-1);
	
	/* compute window */
	for(i = 0; i < size; i++)
	{
		fSum += window[i] = a0 - a1 * std::cos(fConst * i) +
		a2 * std::cos(fConst * 2 * i) - a3 * std::cos(fConst * 3 * i);
	}
	
	/* I do not know why I now need this factor of two */
	fSum = fSum / 2;
	
	/* scale function */
	for (i = 0; i < size; i++)
		window[i] = window[i] / fSum;
}

inline void SpectralAnalysis::generateBlackmanHarris74Window(float *window, int size)
{
	int i;
	float fSum=0;
	for(i=0; i<size; i++)
		fSum += window[i] =
		0.47f - 0.45f*std::cos(float(Globals::two_pi)/(size-1.0f)*i) - 0.01f*std::cos(float(Globals::two_pi)/(size-1.0f)*i*2.0f) - 0.01f*std::cos(float(Globals::two_pi)/(size-1.0f)*i*3.0f);
	fSum = fSum/2;
	for (i = 0; i < size; i++)
		window[i] = window[i] / fSum;
	return;
}

//Whithout Normalization!!
inline void SpectralAnalysis::generateBlackmanHarrisWindow(float *window, int size)
{
  const float a0      = 0.35875f;
  const float a1      = 0.48829f;
  const float a2      = 0.14128f;
  const float a3      = 0.01168f;

  unsigned int idx    = 0;
  while( idx < size )
  {
      window[idx]   = a0 - (a1 * cosf( (Globals::two_pi * idx) / (size - 1) )) + (a2 * cosf( (2.0f * Globals::two_pi * idx) / (size - 1) )) - (a3 * cosf( (3.0f * Globals::two_pi * idx) / (size - 1) ));
      idx++;
  }
  return;
}