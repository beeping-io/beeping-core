#include "Globals.h"
#include <math.h>

namespace Globals
{
  //float durToken = 0.1f; //dur in seconds for each token
  //For mSizeBlockCircularBuffer = 70 > tokenSize = 35frames
  //float durToken = 0.10158730f; //(35*(float)mSpectralAnalysis->mHopSize/mSampleRate) = 0.10158730158730159
  float durToken = 0.104489796f; //(18*(float)mSpectralAnalysis->mHopSize / mSampleRate) = 0.104489796 (when windowssize= 2048 & hopsize=256
  
  //float durFade = 0.075f; //% of token duration for fade in and fadeout
  float durFade = 0.075f; //% of token duration for fade in and fadeout
  float pi = 3.14159265358979323846f;
  float two_pi = 2.f * 3.14159265358979323846f;
  float tokenAmplitude = 0.7f; // -3dB (For second screen)

  //If we want to change this variable and meke them different for each mode then we will need to revise the DecoderAllMultiTone class parameters!!
  int numTokensAll = 32;
  int numTonesAll = 9;

  int numTokensAudible = numTokensAll;
  int numTokensNonAudible = numTokensAll;
  int numTokensHidden = numTokensAll;
  int numTokensCustom = numTokensAll;

  int numTonesAudibleMultiTone = numTonesAll;//was 9, 10 for tones not closer than 2 intervals
  int numTonesNonAudibleMultiTone = numTonesAll;//was 9, 10 for tones not closer than 2 intervals
  int numTonesHiddenMultiTone = numTonesAll;//was 9, 10 for tones not closer than 2 intervals
  int numTonesCustomMultiTone = numTonesAll;//was 9, 10 for tones not closer than 2 intervals

  //int nBinsOffsetForMultiTone = 18;
  //float freqOffsetForMultiTone = 387.597656f; //18.f/mFreq2Bin=387.597656, mFreq2Bin=0.0464399084
  int nBinsOffsetForAudibleMultiTone = 12;
  float freqOffsetForAudibleMultiTone = 258.398442f; //12.f/mFreq2Bin=258.398442491, mFreq2Bin=0.0464399084
                                                       //int nBinsOffsetForNonAudibleMultiTone = 9;
  //float freqOffsetForNonAudibleMultiTone = 193.79883186849696f; //9.f/mFreq2Bin=193.79883186849696, mFreq2Bin=0.0464399084
  //int nBinsOffsetForNonAudibleMultiTone = 7;
  //float freqOffsetForNonAudibleMultiTone = 150.73242478660875f; //7.f/mFreq2Bin=150.73242478660875f, mFreq2Bin=0.0464399084
  //int nBinsOffsetForNonAudibleMultiTone = 6;
  //float freqOffsetForNonAudibleMultiTone = 129.19922124566464f; //6.f/mFreq2Bin=129.19922124566464f, mFreq2Bin=0.0464399084
  //int nBinsOffsetForNonAudibleMultiTone = 5;
  //float freqOffsetForNonAudibleMultiTone = 107.66601770472053f; //5.f/mFreq2Bin=107.66601770472053f, mFreq2Bin=0.0464399084
  int nBinsOffsetForNonAudibleMultiTone = 4;
  float freqOffsetForNonAudibleMultiTone = 86.1328141638f; //4.f/mFreq2Bin=86.1328141638f, mFreq2Bin=0.0464399084
  
  int nBinsOffsetForHiddenMultiTone = 3;
  float freqOffsetForHiddenMultiTone = 64.5996106228f; //3.f/mFreq2Bin=64.5996106228f, mFreq2Bin=0.0464399084
  
  float freqBaseForCustomMultiTone = 12000.f; //default
  int beepsSeparationForCustomMultiTone = 1; //default
  //int nBinsOffsetForCustomMultiTone = 3;
  int nBinsOffsetForCustomMultiTone = 2 + beepsSeparationForCustomMultiTone;
  float freqOffsetForCustomMultiTone = 64.5996106228f; //3.f/mFreq2Bin=64.5996106228f, mFreq2Bin=0.0464399084
  //char frontDoorTokens[2] = {'a', 'f'}; //idx: 10, 15
  //char frontDoorTokens[2] = {'b', 'm'}; //idx: 11, 22
  char frontDoorTokens[2] = {'1', 'o'}; //idx: 2, 24

  int synthMode = 0;
  float synthVolume = 0.f;

  const int numFrontDoorTokens = 2;
  const int numWordTokens = 9;
  const int numCheckTokens = 1;
  const int numCorrectionTokens = 8;
  const int numMessageTokens = numWordTokens + numCheckTokens + numCorrectionTokens; //18 = 9 message (withouth front door tokens) + 1 check code + 8 correction code
  const int numTotalTokens = numFrontDoorTokens + numMessageTokens;

  int init(int fftsize, float samplerate)
  {
    //float freq2Bin = (float)fftsize / samplerate;
    float freq2Bin = (float)fftsize / 44100.f; //We hardcode at 44100.f because we set the separation of beeps in number od bins no matter if we are at 44.1Khz or 48Khz, the decoder will be mostly at 44Khz

    nBinsOffsetForAudibleMultiTone = 12;
    freqOffsetForAudibleMultiTone = (float)nBinsOffsetForAudibleMultiTone / freq2Bin; //12.f/mFreq2Bin=258.398442491, mFreq2Bin=0.0464399084
    
    nBinsOffsetForNonAudibleMultiTone = 4;
    freqOffsetForNonAudibleMultiTone = (float)nBinsOffsetForNonAudibleMultiTone / freq2Bin;
    //freqOffsetForNonAudibleMultiTone = 86.1328141638f; //4.f/mFreq2Bin=86.1328141638f, mFreq2Bin=0.0464399084

    nBinsOffsetForHiddenMultiTone = 3;
    freqOffsetForHiddenMultiTone = (float)nBinsOffsetForHiddenMultiTone / freq2Bin;
    //freqOffsetForHiddenMultiTone = 64.5996106228f; //3.f/mFreq2Bin=64.5996106228f, mFreq2Bin=0.0464399084

    //nBinsOffsetForCustomMultiTone = 3; //should have been set using the function BEEPING_SetCustomBeepsSeparation(...)
    nBinsOffsetForCustomMultiTone = 2 + beepsSeparationForCustomMultiTone;
    freqOffsetForCustomMultiTone = (float)nBinsOffsetForCustomMultiTone / freq2Bin;
    //freqOffsetForCustomMultiTone = 64.5996106228f; //3.f/mFreq2Bin=64.5996106228f, mFreq2Bin=0.0464399084
    
    return 0;
  }

  int getIdxFromChar(char c)
  {
    if (c=='0')                    return  0; //A6
    else if (c=='1')               return  1; //A#6
    else if (c=='2')               return  2; //B6
    else if (c=='3')               return  3; //C7
    else if (c=='4')               return  4; //C#7
    else if (c=='5')               return  5; //D7
    else if (c=='6')               return  6; //D#7
    else if (c=='7')               return  7; //E7
    else if (c=='8')               return  8; //F7
    else if (c=='9')               return  9; //F#7
    else if ((c=='a') || (c=='A')) return 10; //G7
    else if ((c=='b') || (c=='B')) return 11; //G#7
    else if ((c=='c') || (c=='C')) return 12; //A7
    else if ((c=='d') || (c=='D')) return 13; //A#7
    else if ((c=='e') || (c=='E')) return 14; //B7
    else if ((c=='f') || (c=='F')) return 15; //C8
    else if ((c=='g') || (c=='G')) return 16; //C#8
    else if ((c=='h') || (c=='H')) return 17; //D8
    else if ((c=='i') || (c=='I')) return 18; //D#8
    else if ((c=='j') || (c=='J')) return 19; //E8
    else if ((c=='k') || (c=='K')) return 20; //F8
    else if ((c=='l') || (c=='L')) return 21; //F#8
    else if ((c=='m') || (c=='M')) return 22; //G8
    else if ((c=='n') || (c=='N')) return 23; //G#8
    else if ((c=='o') || (c=='O')) return 24; //A8
    else if ((c=='p') || (c=='P')) return 25; //A#8
    else if ((c=='q') || (c=='Q')) return 26; //B8
    else if ((c=='r') || (c=='R')) return 27; //C9
    else if ((c=='s') || (c=='S')) return 28; //C#9
    else if ((c=='t') || (c=='T')) return 29; //D9
    else if ((c=='u') || (c=='U')) return 30; //E9
    else if ((c=='v') || (c=='V')) return 31; //F9
    else                           return -1;
  }

  char getCharFromIdx(int idx)
  {
    if      (idx==0)  return '0'; 
    else if (idx==1)  return '1';
    else if (idx==2)  return '2';
    else if (idx==3)  return '3';
    else if (idx==4)  return '4';
    else if (idx==5)  return '5';
    else if (idx==6)  return '6';
    else if (idx==7)  return '7';
    else if (idx==8)  return '8';
    else if (idx==9)  return '9';
    else if (idx==10) return 'a';
    else if (idx==11) return 'b';
    else if (idx==12) return 'c';
    else if (idx==13) return 'd';
    else if (idx==14) return 'e';
    else if (idx==15) return 'f';
    else if (idx==16) return 'g';
    else if (idx==17) return 'h';
    else if (idx==18) return 'i';
    else if (idx==19) return 'j';
    else if (idx==20) return 'k';
    else if (idx==21) return 'l';
    else if (idx==22) return 'm';
    else if (idx==23) return 'n';
    else if (idx==24) return 'o';
    else if (idx==25) return 'p';
    else if (idx==26) return 'q';
    else if (idx==27) return 'r';
    else if (idx==28) return 's';
    else if (idx==29) return 't';
    else if (idx==30) return 'u';
    else if (idx==31) return 'v';
    else              return '0';
  }


/*  float getFreqFromIdx(int idx)
  {    
    float firstFreq = 1760.f;
    return firstFreq * pow(2.f,(float)idx/12.f);
  } */

  float getFreqFromIdxAudible(int idx, float samplingRate, int windowSize)
  {
    float binToHz = samplingRate / windowSize; // 21,5332Hz for 1 bin at 44100Hz-2048ws

    int firstFreqBin = (int)(3300.f / binToHz + .5); //first token arround 3300Hz
    
    float firstFreq = firstFreqBin * binToHz;

    int tokenDistanceInBins = (int)(210.f / binToHz + .5); //separation between token arround 210Hz

    float tokenDistanceInHz = tokenDistanceInBins * binToHz;

    return firstFreq + idx*tokenDistanceInHz;
  }

  float getFreqFromIdxNonAudible(int idx, float samplingRate, int windowSize)
  {
    float binToHz = samplingRate / windowSize; // 21,5332Hz for 1 bin at 44100Hz-2048ws

    int firstFreqBin = (int)(16800.f / binToHz + .5); //first token arround 3300Hz
    
    float firstFreq = firstFreqBin * binToHz;

    //int tokenDistanceInBins = (int)(210.f / binToHz + .5); //separation between token arround 210Hz
    int tokenDistanceInBins = (int)(155.f / binToHz + .5); //separation between token arround 200Hz

    float tokenDistanceInHz = tokenDistanceInBins * binToHz;

    return firstFreq + idx*tokenDistanceInHz;
  }


  void getIdxsFromIdxAudibleMultiTone(int idx,int **idxtones)
  {
    switch (idx) //http://www.estadisticaparatodos.es/software/misjavascript/javascript_combinatorio2.html
    {
      case 0:  (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;
      case 1:  (*idxtones)[0] = 0;  (*idxtones)[1] = 2;  break;
      case 2:  (*idxtones)[0] = 0;  (*idxtones)[1] = 3;  break;
      case 3:  (*idxtones)[0] = 0;  (*idxtones)[1] = 4;  break;
      case 4:  (*idxtones)[0] = 0;  (*idxtones)[1] = 5;  break;
      case 5:  (*idxtones)[0] = 0;  (*idxtones)[1] = 6;  break;
      case 6:  (*idxtones)[0] = 0;  (*idxtones)[1] = 7;  break;
      case 7:  (*idxtones)[0] = 0;  (*idxtones)[1] = 8;  break;
      case 8:  (*idxtones)[0] = 1;  (*idxtones)[1] = 2;  break;
      case 9:  (*idxtones)[0] = 1;  (*idxtones)[1] = 3;  break;
      case 10: (*idxtones)[0] = 1;  (*idxtones)[1] = 4;  break;
      case 11: (*idxtones)[0] = 1;  (*idxtones)[1] = 5;  break;
      case 12: (*idxtones)[0] = 1;  (*idxtones)[1] = 6;  break;
      case 13: (*idxtones)[0] = 1;  (*idxtones)[1] = 7;  break;
      case 14: (*idxtones)[0] = 1;  (*idxtones)[1] = 8;  break;
      case 15: (*idxtones)[0] = 2;  (*idxtones)[1] = 3;  break;

      case 16: (*idxtones)[0] = 2;  (*idxtones)[1] = 4;  break;
      case 17: (*idxtones)[0] = 2;  (*idxtones)[1] = 5;  break;
      case 18: (*idxtones)[0] = 2;  (*idxtones)[1] = 6;  break;
      case 19: (*idxtones)[0] = 2;  (*idxtones)[1] = 7;  break;
      case 20: (*idxtones)[0] = 2;  (*idxtones)[1] = 8;  break;
      case 21: (*idxtones)[0] = 3;  (*idxtones)[1] = 4;  break;
      case 22: (*idxtones)[0] = 3;  (*idxtones)[1] = 5;  break;
      case 23: (*idxtones)[0] = 3;  (*idxtones)[1] = 6;  break;
      case 24: (*idxtones)[0] = 3;  (*idxtones)[1] = 7;  break;
      case 25: (*idxtones)[0] = 3;  (*idxtones)[1] = 8;  break;
      case 26: (*idxtones)[0] = 4;  (*idxtones)[1] = 5;  break;
      case 27: (*idxtones)[0] = 4;  (*idxtones)[1] = 6;  break;
      case 28: (*idxtones)[0] = 4;  (*idxtones)[1] = 7;  break;
      case 29: (*idxtones)[0] = 4;  (*idxtones)[1] = 8;  break;
      case 30: (*idxtones)[0] = 5;  (*idxtones)[1] = 6;  break;
      case 31: (*idxtones)[0] = 5;  (*idxtones)[1] = 7;  break;
    
      /*case 32: idxtones[0] = 5; idxtones[1] = 8;  break;
      case 33: idxtones[0] = 6; idxtones[1] = 7;  break;
      case 34: idxtones[0] = 6; idxtones[1] = 8;  break;
      case 35: idxtones[0] = 7; idxtones[1] = 8;  break;*/

      default: (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;
    }

    return;
  }

  int getIdxTokenFromIdxsTonesAudibleMultiTone(int idx1, int idx2)
  {
         if ((idx1 == 0) && (idx2 == 1)) return 0;
    else if ((idx1 == 0) && (idx2 == 2)) return 1;
    else if ((idx1 == 0) && (idx2 == 3)) return 2;
    else if ((idx1 == 0) && (idx2 == 4)) return 3;
    else if ((idx1 == 0) && (idx2 == 5)) return 4;
    else if ((idx1 == 0) && (idx2 == 6)) return 5;
    else if ((idx1 == 0) && (idx2 == 7)) return 6;
    else if ((idx1 == 0) && (idx2 == 8)) return 7;
 
    else if ((idx1 == 1) && (idx2 == 2)) return 8;
    else if ((idx1 == 1) && (idx2 == 3)) return 9;
    else if ((idx1 == 1) && (idx2 == 4)) return 10;
    else if ((idx1 == 1) && (idx2 == 5)) return 11;
    else if ((idx1 == 1) && (idx2 == 6)) return 12;
    else if ((idx1 == 1) && (idx2 == 7)) return 13;
    else if ((idx1 == 1) && (idx2 == 8)) return 14;

    else if ((idx1 == 2) && (idx2 == 3)) return 15;
    else if ((idx1 == 2) && (idx2 == 4)) return 16;
    else if ((idx1 == 2) && (idx2 == 5)) return 17;
    else if ((idx1 == 2) && (idx2 == 6)) return 18;
    else if ((idx1 == 2) && (idx2 == 7)) return 19;
    else if ((idx1 == 2) && (idx2 == 8)) return 20;
 
    else if ((idx1 == 3) && (idx2 == 4)) return 21;
    else if ((idx1 == 3) && (idx2 == 5)) return 22;
    else if ((idx1 == 3) && (idx2 == 6)) return 23;
    else if ((idx1 == 3) && (idx2 == 7)) return 24;
    else if ((idx1 == 3) && (idx2 == 8)) return 25;

    else if ((idx1 == 4) && (idx2 == 5)) return 26;
    else if ((idx1 == 4) && (idx2 == 6)) return 27;
    else if ((idx1 == 4) && (idx2 == 7)) return 28;
    else if ((idx1 == 4) && (idx2 == 8)) return 29;

    else if ((idx1 == 5) && (idx2 == 6)) return 30;
    else if ((idx1 == 5) && (idx2 == 7)) return 31;

    //reversed
    else if ((idx2 == 0) && (idx1 == 1)) return 0;
    else if ((idx2 == 0) && (idx1 == 2)) return 1;
    else if ((idx2 == 0) && (idx1 == 3)) return 2;
    else if ((idx2 == 0) && (idx1 == 4)) return 3;
    else if ((idx2 == 0) && (idx1 == 5)) return 4;
    else if ((idx2 == 0) && (idx1 == 6)) return 5;
    else if ((idx2 == 0) && (idx1 == 7)) return 6;
    else if ((idx2 == 0) && (idx1 == 8)) return 7;

    else if ((idx2 == 1) && (idx1 == 2)) return 8;
    else if ((idx2 == 1) && (idx1 == 3)) return 9;
    else if ((idx2 == 1) && (idx1 == 4)) return 10;
    else if ((idx2 == 1) && (idx1 == 5)) return 11;
    else if ((idx2 == 1) && (idx1 == 6)) return 12;
    else if ((idx2 == 1) && (idx1 == 7)) return 13;
    else if ((idx2 == 1) && (idx1 == 8)) return 14;

    else if ((idx2 == 2) && (idx1 == 3)) return 15;
    else if ((idx2 == 2) && (idx1 == 4)) return 16;
    else if ((idx2 == 2) && (idx1 == 5)) return 17;
    else if ((idx2 == 2) && (idx1 == 6)) return 18;
    else if ((idx2 == 2) && (idx1 == 7)) return 19;
    else if ((idx2 == 2) && (idx1 == 8)) return 20;

    else if ((idx2 == 3) && (idx1 == 4)) return 21;
    else if ((idx2 == 3) && (idx1 == 5)) return 22;
    else if ((idx2 == 3) && (idx1 == 6)) return 23;
    else if ((idx2 == 3) && (idx1 == 7)) return 24;
    else if ((idx2 == 3) && (idx1 == 8)) return 25;

    else if ((idx2 == 4) && (idx1 == 5)) return 26;
    else if ((idx2 == 4) && (idx1 == 6)) return 27;
    else if ((idx2 == 4) && (idx1 == 7)) return 28;
    else if ((idx2 == 4) && (idx1 == 8)) return 29;

    else if ((idx2 == 5) && (idx1 == 6)) return 30;
    else if ((idx2 == 5) && (idx1 == 7)) return 31;

    else return -1; //check if this makes sense!

  }

  void getIdxsFromIdxNonAudibleMultiTone(int idx, int **idxtones)
  {
/*
    switch (idx) //http://www.estadisticaparatodos.es/software/misjavascript/javascript_combinatorio2.html
    {
    case 0:  (*idxtones)[0] = 0;  (*idxtones)[1] = 9;  break;
    case 1:  (*idxtones)[0] = 0;  (*idxtones)[1] = 8;  break;
    case 2:  (*idxtones)[0] = 0;  (*idxtones)[1] = 7;  break;
    case 3:  (*idxtones)[0] = 0;  (*idxtones)[1] = 6;  break;
    case 4:  (*idxtones)[0] = 0;  (*idxtones)[1] = 5;  break;
    case 5:  (*idxtones)[0] = 0;  (*idxtones)[1] = 4;  break;
    case 6:  (*idxtones)[0] = 0;  (*idxtones)[1] = 3;  break;
    //case 7:  (*idxtones)[0] = 0;  (*idxtones)[1] = 2;  break;
      case 7:  (*idxtones)[0] = 6;  (*idxtones)[1] = 8;  break;
    case 8:  (*idxtones)[0] = 1;  (*idxtones)[1] = 9;  break;
    case 9:  (*idxtones)[0] = 1;  (*idxtones)[1] = 8;  break;
    case 10: (*idxtones)[0] = 1;  (*idxtones)[1] = 7;  break;
    case 11: (*idxtones)[0] = 1;  (*idxtones)[1] = 6;  break;
    case 12: (*idxtones)[0] = 1;  (*idxtones)[1] = 5;  break;
    case 13: (*idxtones)[0] = 1;  (*idxtones)[1] = 4;  break;
    case 14: (*idxtones)[0] = 1;  (*idxtones)[1] = 3;  break;
    case 15: (*idxtones)[0] = 2;  (*idxtones)[1] = 9;  break;

    case 16: (*idxtones)[0] = 2;  (*idxtones)[1] = 8;  break;
    case 17: (*idxtones)[0] = 2;  (*idxtones)[1] = 7;  break;
    case 18: (*idxtones)[0] = 2;  (*idxtones)[1] = 6;  break;
    case 19: (*idxtones)[0] = 2;  (*idxtones)[1] = 5;  break;
    case 20: (*idxtones)[0] = 2;  (*idxtones)[1] = 4;  break;
    case 21: (*idxtones)[0] = 3;  (*idxtones)[1] = 9;  break;
    case 22: (*idxtones)[0] = 3;  (*idxtones)[1] = 8;  break;
    case 23: (*idxtones)[0] = 3;  (*idxtones)[1] = 7;  break;
    case 24: (*idxtones)[0] = 3;  (*idxtones)[1] = 6;  break;
    case 25: (*idxtones)[0] = 3;  (*idxtones)[1] = 5;  break;
    case 26: (*idxtones)[0] = 4;  (*idxtones)[1] = 9;  break;
    case 27: (*idxtones)[0] = 4;  (*idxtones)[1] = 8;  break;
    case 28: (*idxtones)[0] = 4;  (*idxtones)[1] = 7;  break;
    case 29: (*idxtones)[0] = 4;  (*idxtones)[1] = 6;  break;
    case 30: (*idxtones)[0] = 5;  (*idxtones)[1] = 9;  break;
    case 31: (*idxtones)[0] = 5;  (*idxtones)[1] = 8;  break;

      //case 32: idxtones[0] = 5; idxtones[1] = 7;  break;
      //case 33: idxtones[0] = 6; idxtones[1] = 9;  break;
      //case 34: idxtones[0] = 6; idxtones[1] = 8;  break;//used in 7
      //case 35: idxtones[0] = 7; idxtones[1] = 9;  break;

    default: (*idxtones)[0] = 0;  (*idxtones)[1] = 9;  break;
    
*/
    switch (idx) //http://www.estadisticaparatodos.es/software/misjavascript/javascript_combinatorio2.html
    {
    case 0:  (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;//commented for more distance between freqs
    //  case 0:  (*idxtones)[0] = 5;  (*idxtones)[1] = 8;  break;//
    case 1:  (*idxtones)[0] = 0;  (*idxtones)[1] = 2;  break;
    case 2:  (*idxtones)[0] = 0;  (*idxtones)[1] = 3;  break;
    case 3:  (*idxtones)[0] = 0;  (*idxtones)[1] = 4;  break;
    case 4:  (*idxtones)[0] = 0;  (*idxtones)[1] = 5;  break;
    case 5:  (*idxtones)[0] = 0;  (*idxtones)[1] = 6;  break;
    case 6:  (*idxtones)[0] = 0;  (*idxtones)[1] = 7;  break;
    case 7:  (*idxtones)[0] = 0;  (*idxtones)[1] = 8;  break;
    case 8:  (*idxtones)[0] = 1;  (*idxtones)[1] = 2;  break;
    case 9:  (*idxtones)[0] = 1;  (*idxtones)[1] = 3;  break;
    case 10: (*idxtones)[0] = 1;  (*idxtones)[1] = 4;  break;
    case 11: (*idxtones)[0] = 1;  (*idxtones)[1] = 5;  break;
    case 12: (*idxtones)[0] = 1;  (*idxtones)[1] = 6;  break;
    case 13: (*idxtones)[0] = 1;  (*idxtones)[1] = 7;  break;
    case 14: (*idxtones)[0] = 1;  (*idxtones)[1] = 8;  break;
    case 15: (*idxtones)[0] = 2;  (*idxtones)[1] = 3;  break;

    case 16: (*idxtones)[0] = 2;  (*idxtones)[1] = 4;  break;
    case 17: (*idxtones)[0] = 2;  (*idxtones)[1] = 5;  break;
    case 18: (*idxtones)[0] = 2;  (*idxtones)[1] = 6;  break;
    case 19: (*idxtones)[0] = 2;  (*idxtones)[1] = 7;  break;
    case 20: (*idxtones)[0] = 2;  (*idxtones)[1] = 8;  break;
    case 21: (*idxtones)[0] = 3;  (*idxtones)[1] = 4;  break;
    case 22: (*idxtones)[0] = 3;  (*idxtones)[1] = 5;  break;
    case 23: (*idxtones)[0] = 3;  (*idxtones)[1] = 6;  break;
    case 24: (*idxtones)[0] = 3;  (*idxtones)[1] = 7;  break;
    case 25: (*idxtones)[0] = 3;  (*idxtones)[1] = 8;  break;
    case 26: (*idxtones)[0] = 4;  (*idxtones)[1] = 5;  break;
    case 27: (*idxtones)[0] = 4;  (*idxtones)[1] = 6;  break;
    case 28: (*idxtones)[0] = 4;  (*idxtones)[1] = 7;  break;
    case 29: (*idxtones)[0] = 4;  (*idxtones)[1] = 8;  break;
    case 30: (*idxtones)[0] = 5;  (*idxtones)[1] = 6;  break;
    case 31: (*idxtones)[0] = 5;  (*idxtones)[1] = 7;  break;

      //case 32: idxtones[0] = 5; idxtones[1] = 8;  break;
      //case 33: idxtones[0] = 6; idxtones[1] = 7;  break;
      //case 34: idxtones[0] = 6; idxtones[1] = 8;  break;
      //case 35: idxtones[0] = 7; idxtones[1] = 8;  break;

    default: (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;
    }

    return;
  }

  void getIdxsFromIdxHiddenMultiTone(int idx, int **idxtones)
  {
    /*
    switch (idx) //http://www.estadisticaparatodos.es/software/misjavascript/javascript_combinatorio2.html
    {
    case 0:  (*idxtones)[0] = 0;  (*idxtones)[1] = 9;  break;
    case 1:  (*idxtones)[0] = 0;  (*idxtones)[1] = 8;  break;
    case 2:  (*idxtones)[0] = 0;  (*idxtones)[1] = 7;  break;
    case 3:  (*idxtones)[0] = 0;  (*idxtones)[1] = 6;  break;
    case 4:  (*idxtones)[0] = 0;  (*idxtones)[1] = 5;  break;
    case 5:  (*idxtones)[0] = 0;  (*idxtones)[1] = 4;  break;
    case 6:  (*idxtones)[0] = 0;  (*idxtones)[1] = 3;  break;
    //case 7:  (*idxtones)[0] = 0;  (*idxtones)[1] = 2;  break;
    case 7:  (*idxtones)[0] = 6;  (*idxtones)[1] = 8;  break;
    case 8:  (*idxtones)[0] = 1;  (*idxtones)[1] = 9;  break;
    case 9:  (*idxtones)[0] = 1;  (*idxtones)[1] = 8;  break;
    case 10: (*idxtones)[0] = 1;  (*idxtones)[1] = 7;  break;
    case 11: (*idxtones)[0] = 1;  (*idxtones)[1] = 6;  break;
    case 12: (*idxtones)[0] = 1;  (*idxtones)[1] = 5;  break;
    case 13: (*idxtones)[0] = 1;  (*idxtones)[1] = 4;  break;
    case 14: (*idxtones)[0] = 1;  (*idxtones)[1] = 3;  break;
    case 15: (*idxtones)[0] = 2;  (*idxtones)[1] = 9;  break;

    case 16: (*idxtones)[0] = 2;  (*idxtones)[1] = 8;  break;
    case 17: (*idxtones)[0] = 2;  (*idxtones)[1] = 7;  break;
    case 18: (*idxtones)[0] = 2;  (*idxtones)[1] = 6;  break;
    case 19: (*idxtones)[0] = 2;  (*idxtones)[1] = 5;  break;
    case 20: (*idxtones)[0] = 2;  (*idxtones)[1] = 4;  break;
    case 21: (*idxtones)[0] = 3;  (*idxtones)[1] = 9;  break;
    case 22: (*idxtones)[0] = 3;  (*idxtones)[1] = 8;  break;
    case 23: (*idxtones)[0] = 3;  (*idxtones)[1] = 7;  break;
    case 24: (*idxtones)[0] = 3;  (*idxtones)[1] = 6;  break;
    case 25: (*idxtones)[0] = 3;  (*idxtones)[1] = 5;  break;
    case 26: (*idxtones)[0] = 4;  (*idxtones)[1] = 9;  break;
    case 27: (*idxtones)[0] = 4;  (*idxtones)[1] = 8;  break;
    case 28: (*idxtones)[0] = 4;  (*idxtones)[1] = 7;  break;
    case 29: (*idxtones)[0] = 4;  (*idxtones)[1] = 6;  break;
    case 30: (*idxtones)[0] = 5;  (*idxtones)[1] = 9;  break;
    case 31: (*idxtones)[0] = 5;  (*idxtones)[1] = 8;  break;

    //case 32: idxtones[0] = 5; idxtones[1] = 7;  break;
    //case 33: idxtones[0] = 6; idxtones[1] = 9;  break;
    //case 34: idxtones[0] = 6; idxtones[1] = 8;  break;//used in 7
    //case 35: idxtones[0] = 7; idxtones[1] = 9;  break;

    default: (*idxtones)[0] = 0;  (*idxtones)[1] = 9;  break;

    */
    switch (idx) //http://www.estadisticaparatodos.es/software/misjavascript/javascript_combinatorio2.html
    {
    case 0:  (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;//commented for more distance between freqs
                                                             //  case 0:  (*idxtones)[0] = 5;  (*idxtones)[1] = 8;  break;//
    case 1:  (*idxtones)[0] = 0;  (*idxtones)[1] = 2;  break;
    case 2:  (*idxtones)[0] = 0;  (*idxtones)[1] = 3;  break;
    case 3:  (*idxtones)[0] = 0;  (*idxtones)[1] = 4;  break;
    case 4:  (*idxtones)[0] = 0;  (*idxtones)[1] = 5;  break;
    case 5:  (*idxtones)[0] = 0;  (*idxtones)[1] = 6;  break;
    case 6:  (*idxtones)[0] = 0;  (*idxtones)[1] = 7;  break;
    case 7:  (*idxtones)[0] = 0;  (*idxtones)[1] = 8;  break;
    case 8:  (*idxtones)[0] = 1;  (*idxtones)[1] = 2;  break;
    case 9:  (*idxtones)[0] = 1;  (*idxtones)[1] = 3;  break;
    case 10: (*idxtones)[0] = 1;  (*idxtones)[1] = 4;  break;
    case 11: (*idxtones)[0] = 1;  (*idxtones)[1] = 5;  break;
    case 12: (*idxtones)[0] = 1;  (*idxtones)[1] = 6;  break;
    case 13: (*idxtones)[0] = 1;  (*idxtones)[1] = 7;  break;
    case 14: (*idxtones)[0] = 1;  (*idxtones)[1] = 8;  break;
    case 15: (*idxtones)[0] = 2;  (*idxtones)[1] = 3;  break;

    case 16: (*idxtones)[0] = 2;  (*idxtones)[1] = 4;  break;
    case 17: (*idxtones)[0] = 2;  (*idxtones)[1] = 5;  break;
    case 18: (*idxtones)[0] = 2;  (*idxtones)[1] = 6;  break;
    case 19: (*idxtones)[0] = 2;  (*idxtones)[1] = 7;  break;
    case 20: (*idxtones)[0] = 2;  (*idxtones)[1] = 8;  break;
    case 21: (*idxtones)[0] = 3;  (*idxtones)[1] = 4;  break;
    case 22: (*idxtones)[0] = 3;  (*idxtones)[1] = 5;  break;
    case 23: (*idxtones)[0] = 3;  (*idxtones)[1] = 6;  break;
    case 24: (*idxtones)[0] = 3;  (*idxtones)[1] = 7;  break;
    case 25: (*idxtones)[0] = 3;  (*idxtones)[1] = 8;  break;
    case 26: (*idxtones)[0] = 4;  (*idxtones)[1] = 5;  break;
    case 27: (*idxtones)[0] = 4;  (*idxtones)[1] = 6;  break;
    case 28: (*idxtones)[0] = 4;  (*idxtones)[1] = 7;  break;
    case 29: (*idxtones)[0] = 4;  (*idxtones)[1] = 8;  break;
    case 30: (*idxtones)[0] = 5;  (*idxtones)[1] = 6;  break;
    case 31: (*idxtones)[0] = 5;  (*idxtones)[1] = 7;  break;

      //case 32: idxtones[0] = 5; idxtones[1] = 8;  break;
      //case 33: idxtones[0] = 6; idxtones[1] = 7;  break;
      //case 34: idxtones[0] = 6; idxtones[1] = 8;  break;
      //case 35: idxtones[0] = 7; idxtones[1] = 8;  break;

    default: (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;
    }

    return;
  }

  void getIdxsFromIdxCustomMultiTone(int idx, int **idxtones)
  {
    /*
    switch (idx) //http://www.estadisticaparatodos.es/software/misjavascript/javascript_combinatorio2.html
    {
    case 0:  (*idxtones)[0] = 0;  (*idxtones)[1] = 9;  break;
    case 1:  (*idxtones)[0] = 0;  (*idxtones)[1] = 8;  break;
    case 2:  (*idxtones)[0] = 0;  (*idxtones)[1] = 7;  break;
    case 3:  (*idxtones)[0] = 0;  (*idxtones)[1] = 6;  break;
    case 4:  (*idxtones)[0] = 0;  (*idxtones)[1] = 5;  break;
    case 5:  (*idxtones)[0] = 0;  (*idxtones)[1] = 4;  break;
    case 6:  (*idxtones)[0] = 0;  (*idxtones)[1] = 3;  break;
    //case 7:  (*idxtones)[0] = 0;  (*idxtones)[1] = 2;  break;
    case 7:  (*idxtones)[0] = 6;  (*idxtones)[1] = 8;  break;
    case 8:  (*idxtones)[0] = 1;  (*idxtones)[1] = 9;  break;
    case 9:  (*idxtones)[0] = 1;  (*idxtones)[1] = 8;  break;
    case 10: (*idxtones)[0] = 1;  (*idxtones)[1] = 7;  break;
    case 11: (*idxtones)[0] = 1;  (*idxtones)[1] = 6;  break;
    case 12: (*idxtones)[0] = 1;  (*idxtones)[1] = 5;  break;
    case 13: (*idxtones)[0] = 1;  (*idxtones)[1] = 4;  break;
    case 14: (*idxtones)[0] = 1;  (*idxtones)[1] = 3;  break;
    case 15: (*idxtones)[0] = 2;  (*idxtones)[1] = 9;  break;

    case 16: (*idxtones)[0] = 2;  (*idxtones)[1] = 8;  break;
    case 17: (*idxtones)[0] = 2;  (*idxtones)[1] = 7;  break;
    case 18: (*idxtones)[0] = 2;  (*idxtones)[1] = 6;  break;
    case 19: (*idxtones)[0] = 2;  (*idxtones)[1] = 5;  break;
    case 20: (*idxtones)[0] = 2;  (*idxtones)[1] = 4;  break;
    case 21: (*idxtones)[0] = 3;  (*idxtones)[1] = 9;  break;
    case 22: (*idxtones)[0] = 3;  (*idxtones)[1] = 8;  break;
    case 23: (*idxtones)[0] = 3;  (*idxtones)[1] = 7;  break;
    case 24: (*idxtones)[0] = 3;  (*idxtones)[1] = 6;  break;
    case 25: (*idxtones)[0] = 3;  (*idxtones)[1] = 5;  break;
    case 26: (*idxtones)[0] = 4;  (*idxtones)[1] = 9;  break;
    case 27: (*idxtones)[0] = 4;  (*idxtones)[1] = 8;  break;
    case 28: (*idxtones)[0] = 4;  (*idxtones)[1] = 7;  break;
    case 29: (*idxtones)[0] = 4;  (*idxtones)[1] = 6;  break;
    case 30: (*idxtones)[0] = 5;  (*idxtones)[1] = 9;  break;
    case 31: (*idxtones)[0] = 5;  (*idxtones)[1] = 8;  break;

    //case 32: idxtones[0] = 5; idxtones[1] = 7;  break;
    //case 33: idxtones[0] = 6; idxtones[1] = 9;  break;
    //case 34: idxtones[0] = 6; idxtones[1] = 8;  break;//used in 7
    //case 35: idxtones[0] = 7; idxtones[1] = 9;  break;

    default: (*idxtones)[0] = 0;  (*idxtones)[1] = 9;  break;

    */
    switch (idx) //http://www.estadisticaparatodos.es/software/misjavascript/javascript_combinatorio2.html
    {
    case 0:  (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;//commented for more distance between freqs
                                                             //  case 0:  (*idxtones)[0] = 5;  (*idxtones)[1] = 8;  break;//
    case 1:  (*idxtones)[0] = 0;  (*idxtones)[1] = 2;  break;
    case 2:  (*idxtones)[0] = 0;  (*idxtones)[1] = 3;  break;
    case 3:  (*idxtones)[0] = 0;  (*idxtones)[1] = 4;  break;
    case 4:  (*idxtones)[0] = 0;  (*idxtones)[1] = 5;  break;
    case 5:  (*idxtones)[0] = 0;  (*idxtones)[1] = 6;  break;
    case 6:  (*idxtones)[0] = 0;  (*idxtones)[1] = 7;  break;
    case 7:  (*idxtones)[0] = 0;  (*idxtones)[1] = 8;  break;
    case 8:  (*idxtones)[0] = 1;  (*idxtones)[1] = 2;  break;
    case 9:  (*idxtones)[0] = 1;  (*idxtones)[1] = 3;  break;
    case 10: (*idxtones)[0] = 1;  (*idxtones)[1] = 4;  break;
    case 11: (*idxtones)[0] = 1;  (*idxtones)[1] = 5;  break;
    case 12: (*idxtones)[0] = 1;  (*idxtones)[1] = 6;  break;
    case 13: (*idxtones)[0] = 1;  (*idxtones)[1] = 7;  break;
    case 14: (*idxtones)[0] = 1;  (*idxtones)[1] = 8;  break;
    case 15: (*idxtones)[0] = 2;  (*idxtones)[1] = 3;  break;

    case 16: (*idxtones)[0] = 2;  (*idxtones)[1] = 4;  break;
    case 17: (*idxtones)[0] = 2;  (*idxtones)[1] = 5;  break;
    case 18: (*idxtones)[0] = 2;  (*idxtones)[1] = 6;  break;
    case 19: (*idxtones)[0] = 2;  (*idxtones)[1] = 7;  break;
    case 20: (*idxtones)[0] = 2;  (*idxtones)[1] = 8;  break;
    case 21: (*idxtones)[0] = 3;  (*idxtones)[1] = 4;  break;
    case 22: (*idxtones)[0] = 3;  (*idxtones)[1] = 5;  break;
    case 23: (*idxtones)[0] = 3;  (*idxtones)[1] = 6;  break;
    case 24: (*idxtones)[0] = 3;  (*idxtones)[1] = 7;  break;
    case 25: (*idxtones)[0] = 3;  (*idxtones)[1] = 8;  break;
    case 26: (*idxtones)[0] = 4;  (*idxtones)[1] = 5;  break;
    case 27: (*idxtones)[0] = 4;  (*idxtones)[1] = 6;  break;
    case 28: (*idxtones)[0] = 4;  (*idxtones)[1] = 7;  break;
    case 29: (*idxtones)[0] = 4;  (*idxtones)[1] = 8;  break;
    case 30: (*idxtones)[0] = 5;  (*idxtones)[1] = 6;  break;
    case 31: (*idxtones)[0] = 5;  (*idxtones)[1] = 7;  break;

      //case 32: idxtones[0] = 5; idxtones[1] = 8;  break;
      //case 33: idxtones[0] = 6; idxtones[1] = 7;  break;
      //case 34: idxtones[0] = 6; idxtones[1] = 8;  break;
      //case 35: idxtones[0] = 7; idxtones[1] = 8;  break;

    default: (*idxtones)[0] = 0;  (*idxtones)[1] = 1;  break;
    }

    return;
  }

  int getIdxTokenFromIdxsTonesNonAudibleMultiTone(int idx1, int idx2)
  {
/*    if ((idx1 == 0) && (idx2 == 9)) return 0;
    else if ((idx1 == 0) && (idx2 == 8)) return 1;
    else if ((idx1 == 0) && (idx2 == 7)) return 2;
    else if ((idx1 == 0) && (idx2 == 6)) return 3;
    else if ((idx1 == 0) && (idx2 == 5)) return 4;
    else if ((idx1 == 0) && (idx2 == 4)) return 5;
    else if ((idx1 == 0) && (idx2 == 3)) return 6;
    //else if ((idx1 == 0) && (idx2 == 2)) return 7;
    else if ((idx1 == 6) && (idx2 == 8)) return 7;

    else if ((idx1 == 1) && (idx2 == 9)) return 8;
    else if ((idx1 == 1) && (idx2 == 8)) return 9;
    else if ((idx1 == 1) && (idx2 == 7)) return 10;
    else if ((idx1 == 1) && (idx2 == 6)) return 11;
    else if ((idx1 == 1) && (idx2 == 5)) return 12;
    else if ((idx1 == 1) && (idx2 == 4)) return 13;
    else if ((idx1 == 1) && (idx2 == 3)) return 14;

    else if ((idx1 == 2) && (idx2 == 9)) return 15;
    else if ((idx1 == 2) && (idx2 == 8)) return 16;
    else if ((idx1 == 2) && (idx2 == 7)) return 17;
    else if ((idx1 == 2) && (idx2 == 6)) return 18;
    else if ((idx1 == 2) && (idx2 == 5)) return 19;
    else if ((idx1 == 2) && (idx2 == 4)) return 20;

    else if ((idx1 == 3) && (idx2 == 9)) return 21;
    else if ((idx1 == 3) && (idx2 == 8)) return 22;
    else if ((idx1 == 3) && (idx2 == 7)) return 23;
    else if ((idx1 == 3) && (idx2 == 6)) return 24;
    else if ((idx1 == 3) && (idx2 == 5)) return 25;

    else if ((idx1 == 4) && (idx2 == 9)) return 26;
    else if ((idx1 == 4) && (idx2 == 8)) return 27;
    else if ((idx1 == 4) && (idx2 == 7)) return 28;
    else if ((idx1 == 4) && (idx2 == 6)) return 29;

    else if ((idx1 == 5) && (idx2 == 9)) return 30;
    else if ((idx1 == 5) && (idx2 == 8)) return 31;

    //reversed
    else if ((idx2 == 0) && (idx1 == 9)) return 0;
    else if ((idx2 == 0) && (idx1 == 8)) return 1;
    else if ((idx2 == 0) && (idx1 == 7)) return 2;
    else if ((idx2 == 0) && (idx1 == 6)) return 3;
    else if ((idx2 == 0) && (idx1 == 5)) return 4;
    else if ((idx2 == 0) && (idx1 == 4)) return 5;
    else if ((idx2 == 0) && (idx1 == 3)) return 6;
    //else if ((idx2 == 0) && (idx1 == 2)) return 7;
    else if ((idx2 == 6) && (idx1 == 8)) return 7;

    else if ((idx2 == 1) && (idx1 == 9)) return 8;
    else if ((idx2 == 1) && (idx1 == 8)) return 9;
    else if ((idx2 == 1) && (idx1 == 7)) return 10;
    else if ((idx2 == 1) && (idx1 == 6)) return 11;
    else if ((idx2 == 1) && (idx1 == 5)) return 12;
    else if ((idx2 == 1) && (idx1 == 4)) return 13;
    else if ((idx2 == 1) && (idx1 == 3)) return 14;

    else if ((idx2 == 2) && (idx1 == 9)) return 15;
    else if ((idx2 == 2) && (idx1 == 8)) return 16;
    else if ((idx2 == 2) && (idx1 == 7)) return 17;
    else if ((idx2 == 2) && (idx1 == 6)) return 18;
    else if ((idx2 == 2) && (idx1 == 5)) return 19;
    else if ((idx2 == 2) && (idx1 == 4)) return 20;

    else if ((idx2 == 3) && (idx1 == 9)) return 21;
    else if ((idx2 == 3) && (idx1 == 8)) return 22;
    else if ((idx2 == 3) && (idx1 == 7)) return 23;
    else if ((idx2 == 3) && (idx1 == 6)) return 24;
    else if ((idx2 == 3) && (idx1 == 5)) return 25;

    else if ((idx2 == 4) && (idx1 == 9)) return 26;
    else if ((idx2 == 4) && (idx1 == 8)) return 27;
    else if ((idx2 == 4) && (idx1 == 7)) return 28;
    else if ((idx2 == 4) && (idx1 == 6)) return 29;

    else if ((idx2 == 5) && (idx1 == 9)) return 30;
    else if ((idx2 == 5) && (idx1 == 8)) return 31;

    else return 0; //check if this makes sense!
*/


    if ((idx1 == 0) && (idx2 == 1)) return 0;
    else if ((idx1 == 0) && (idx2 == 2)) return 1;
    else if ((idx1 == 0) && (idx2 == 3)) return 2;
    else if ((idx1 == 0) && (idx2 == 4)) return 3;
    else if ((idx1 == 0) && (idx2 == 5)) return 4;
    else if ((idx1 == 0) && (idx2 == 6)) return 5;
    else if ((idx1 == 0) && (idx2 == 7)) return 6;
    else if ((idx1 == 0) && (idx2 == 8)) return 7;

    else if ((idx1 == 1) && (idx2 == 2)) return 8;
    else if ((idx1 == 1) && (idx2 == 3)) return 9;
    else if ((idx1 == 1) && (idx2 == 4)) return 10;
    else if ((idx1 == 1) && (idx2 == 5)) return 11;
    else if ((idx1 == 1) && (idx2 == 6)) return 12;
    else if ((idx1 == 1) && (idx2 == 7)) return 13;
    else if ((idx1 == 1) && (idx2 == 8)) return 14;

    else if ((idx1 == 2) && (idx2 == 3)) return 15;
    else if ((idx1 == 2) && (idx2 == 4)) return 16;
    else if ((idx1 == 2) && (idx2 == 5)) return 17;
    else if ((idx1 == 2) && (idx2 == 6)) return 18;
    else if ((idx1 == 2) && (idx2 == 7)) return 19;
    else if ((idx1 == 2) && (idx2 == 8)) return 20;

    else if ((idx1 == 3) && (idx2 == 4)) return 21;
    else if ((idx1 == 3) && (idx2 == 5)) return 22;
    else if ((idx1 == 3) && (idx2 == 6)) return 23;
    else if ((idx1 == 3) && (idx2 == 7)) return 24;
    else if ((idx1 == 3) && (idx2 == 8)) return 25;

    else if ((idx1 == 4) && (idx2 == 5)) return 26;
    else if ((idx1 == 4) && (idx2 == 6)) return 27;
    else if ((idx1 == 4) && (idx2 == 7)) return 28;
    else if ((idx1 == 4) && (idx2 == 8)) return 29;

    else if ((idx1 == 5) && (idx2 == 6)) return 30;
    else if ((idx1 == 5) && (idx2 == 7)) return 31;

    //reversed
    else if ((idx2 == 0) && (idx1 == 1)) return 0;
    else if ((idx2 == 0) && (idx1 == 2)) return 1;
    else if ((idx2 == 0) && (idx1 == 3)) return 2;
    else if ((idx2 == 0) && (idx1 == 4)) return 3;
    else if ((idx2 == 0) && (idx1 == 5)) return 4;
    else if ((idx2 == 0) && (idx1 == 6)) return 5;
    else if ((idx2 == 0) && (idx1 == 7)) return 6;
    else if ((idx2 == 0) && (idx1 == 8)) return 7;

    else if ((idx2 == 1) && (idx1 == 2)) return 8;
    else if ((idx2 == 1) && (idx1 == 3)) return 9;
    else if ((idx2 == 1) && (idx1 == 4)) return 10;
    else if ((idx2 == 1) && (idx1 == 5)) return 11;
    else if ((idx2 == 1) && (idx1 == 6)) return 12;
    else if ((idx2 == 1) && (idx1 == 7)) return 13;
    else if ((idx2 == 1) && (idx1 == 8)) return 14;

    else if ((idx2 == 2) && (idx1 == 3)) return 15;
    else if ((idx2 == 2) && (idx1 == 4)) return 16;
    else if ((idx2 == 2) && (idx1 == 5)) return 17;
    else if ((idx2 == 2) && (idx1 == 6)) return 18;
    else if ((idx2 == 2) && (idx1 == 7)) return 19;
    else if ((idx2 == 2) && (idx1 == 8)) return 20;

    else if ((idx2 == 3) && (idx1 == 4)) return 21;
    else if ((idx2 == 3) && (idx1 == 5)) return 22;
    else if ((idx2 == 3) && (idx1 == 6)) return 23;
    else if ((idx2 == 3) && (idx1 == 7)) return 24;
    else if ((idx2 == 3) && (idx1 == 8)) return 25;

    else if ((idx2 == 4) && (idx1 == 5)) return 26;
    else if ((idx2 == 4) && (idx1 == 6)) return 27;
    else if ((idx2 == 4) && (idx1 == 7)) return 28;
    else if ((idx2 == 4) && (idx1 == 8)) return 29;

    else if ((idx2 == 5) && (idx1 == 6)) return 30;
    else if ((idx2 == 5) && (idx1 == 7)) return 31;

    else return -1; //check if this makes sense!

  }


  int getIdxTokenFromIdxsTonesHiddenMultiTone(int idx1, int idx2)
  {
    /*    if ((idx1 == 0) && (idx2 == 9)) return 0;
    else if ((idx1 == 0) && (idx2 == 8)) return 1;
    else if ((idx1 == 0) && (idx2 == 7)) return 2;
    else if ((idx1 == 0) && (idx2 == 6)) return 3;
    else if ((idx1 == 0) && (idx2 == 5)) return 4;
    else if ((idx1 == 0) && (idx2 == 4)) return 5;
    else if ((idx1 == 0) && (idx2 == 3)) return 6;
    //else if ((idx1 == 0) && (idx2 == 2)) return 7;
    else if ((idx1 == 6) && (idx2 == 8)) return 7;

    else if ((idx1 == 1) && (idx2 == 9)) return 8;
    else if ((idx1 == 1) && (idx2 == 8)) return 9;
    else if ((idx1 == 1) && (idx2 == 7)) return 10;
    else if ((idx1 == 1) && (idx2 == 6)) return 11;
    else if ((idx1 == 1) && (idx2 == 5)) return 12;
    else if ((idx1 == 1) && (idx2 == 4)) return 13;
    else if ((idx1 == 1) && (idx2 == 3)) return 14;

    else if ((idx1 == 2) && (idx2 == 9)) return 15;
    else if ((idx1 == 2) && (idx2 == 8)) return 16;
    else if ((idx1 == 2) && (idx2 == 7)) return 17;
    else if ((idx1 == 2) && (idx2 == 6)) return 18;
    else if ((idx1 == 2) && (idx2 == 5)) return 19;
    else if ((idx1 == 2) && (idx2 == 4)) return 20;

    else if ((idx1 == 3) && (idx2 == 9)) return 21;
    else if ((idx1 == 3) && (idx2 == 8)) return 22;
    else if ((idx1 == 3) && (idx2 == 7)) return 23;
    else if ((idx1 == 3) && (idx2 == 6)) return 24;
    else if ((idx1 == 3) && (idx2 == 5)) return 25;

    else if ((idx1 == 4) && (idx2 == 9)) return 26;
    else if ((idx1 == 4) && (idx2 == 8)) return 27;
    else if ((idx1 == 4) && (idx2 == 7)) return 28;
    else if ((idx1 == 4) && (idx2 == 6)) return 29;

    else if ((idx1 == 5) && (idx2 == 9)) return 30;
    else if ((idx1 == 5) && (idx2 == 8)) return 31;

    //reversed
    else if ((idx2 == 0) && (idx1 == 9)) return 0;
    else if ((idx2 == 0) && (idx1 == 8)) return 1;
    else if ((idx2 == 0) && (idx1 == 7)) return 2;
    else if ((idx2 == 0) && (idx1 == 6)) return 3;
    else if ((idx2 == 0) && (idx1 == 5)) return 4;
    else if ((idx2 == 0) && (idx1 == 4)) return 5;
    else if ((idx2 == 0) && (idx1 == 3)) return 6;
    //else if ((idx2 == 0) && (idx1 == 2)) return 7;
    else if ((idx2 == 6) && (idx1 == 8)) return 7;

    else if ((idx2 == 1) && (idx1 == 9)) return 8;
    else if ((idx2 == 1) && (idx1 == 8)) return 9;
    else if ((idx2 == 1) && (idx1 == 7)) return 10;
    else if ((idx2 == 1) && (idx1 == 6)) return 11;
    else if ((idx2 == 1) && (idx1 == 5)) return 12;
    else if ((idx2 == 1) && (idx1 == 4)) return 13;
    else if ((idx2 == 1) && (idx1 == 3)) return 14;

    else if ((idx2 == 2) && (idx1 == 9)) return 15;
    else if ((idx2 == 2) && (idx1 == 8)) return 16;
    else if ((idx2 == 2) && (idx1 == 7)) return 17;
    else if ((idx2 == 2) && (idx1 == 6)) return 18;
    else if ((idx2 == 2) && (idx1 == 5)) return 19;
    else if ((idx2 == 2) && (idx1 == 4)) return 20;

    else if ((idx2 == 3) && (idx1 == 9)) return 21;
    else if ((idx2 == 3) && (idx1 == 8)) return 22;
    else if ((idx2 == 3) && (idx1 == 7)) return 23;
    else if ((idx2 == 3) && (idx1 == 6)) return 24;
    else if ((idx2 == 3) && (idx1 == 5)) return 25;

    else if ((idx2 == 4) && (idx1 == 9)) return 26;
    else if ((idx2 == 4) && (idx1 == 8)) return 27;
    else if ((idx2 == 4) && (idx1 == 7)) return 28;
    else if ((idx2 == 4) && (idx1 == 6)) return 29;

    else if ((idx2 == 5) && (idx1 == 9)) return 30;
    else if ((idx2 == 5) && (idx1 == 8)) return 31;

    else return 0; //check if this makes sense!
    */


    if ((idx1 == 0) && (idx2 == 1)) return 0;
    else if ((idx1 == 0) && (idx2 == 2)) return 1;
    else if ((idx1 == 0) && (idx2 == 3)) return 2;
    else if ((idx1 == 0) && (idx2 == 4)) return 3;
    else if ((idx1 == 0) && (idx2 == 5)) return 4;
    else if ((idx1 == 0) && (idx2 == 6)) return 5;
    else if ((idx1 == 0) && (idx2 == 7)) return 6;
    else if ((idx1 == 0) && (idx2 == 8)) return 7;

    else if ((idx1 == 1) && (idx2 == 2)) return 8;
    else if ((idx1 == 1) && (idx2 == 3)) return 9;
    else if ((idx1 == 1) && (idx2 == 4)) return 10;
    else if ((idx1 == 1) && (idx2 == 5)) return 11;
    else if ((idx1 == 1) && (idx2 == 6)) return 12;
    else if ((idx1 == 1) && (idx2 == 7)) return 13;
    else if ((idx1 == 1) && (idx2 == 8)) return 14;

    else if ((idx1 == 2) && (idx2 == 3)) return 15;
    else if ((idx1 == 2) && (idx2 == 4)) return 16;
    else if ((idx1 == 2) && (idx2 == 5)) return 17;
    else if ((idx1 == 2) && (idx2 == 6)) return 18;
    else if ((idx1 == 2) && (idx2 == 7)) return 19;
    else if ((idx1 == 2) && (idx2 == 8)) return 20;

    else if ((idx1 == 3) && (idx2 == 4)) return 21;
    else if ((idx1 == 3) && (idx2 == 5)) return 22;
    else if ((idx1 == 3) && (idx2 == 6)) return 23;
    else if ((idx1 == 3) && (idx2 == 7)) return 24;
    else if ((idx1 == 3) && (idx2 == 8)) return 25;

    else if ((idx1 == 4) && (idx2 == 5)) return 26;
    else if ((idx1 == 4) && (idx2 == 6)) return 27;
    else if ((idx1 == 4) && (idx2 == 7)) return 28;
    else if ((idx1 == 4) && (idx2 == 8)) return 29;

    else if ((idx1 == 5) && (idx2 == 6)) return 30;
    else if ((idx1 == 5) && (idx2 == 7)) return 31;

    //reversed
    else if ((idx2 == 0) && (idx1 == 1)) return 0;
    else if ((idx2 == 0) && (idx1 == 2)) return 1;
    else if ((idx2 == 0) && (idx1 == 3)) return 2;
    else if ((idx2 == 0) && (idx1 == 4)) return 3;
    else if ((idx2 == 0) && (idx1 == 5)) return 4;
    else if ((idx2 == 0) && (idx1 == 6)) return 5;
    else if ((idx2 == 0) && (idx1 == 7)) return 6;
    else if ((idx2 == 0) && (idx1 == 8)) return 7;

    else if ((idx2 == 1) && (idx1 == 2)) return 8;
    else if ((idx2 == 1) && (idx1 == 3)) return 9;
    else if ((idx2 == 1) && (idx1 == 4)) return 10;
    else if ((idx2 == 1) && (idx1 == 5)) return 11;
    else if ((idx2 == 1) && (idx1 == 6)) return 12;
    else if ((idx2 == 1) && (idx1 == 7)) return 13;
    else if ((idx2 == 1) && (idx1 == 8)) return 14;

    else if ((idx2 == 2) && (idx1 == 3)) return 15;
    else if ((idx2 == 2) && (idx1 == 4)) return 16;
    else if ((idx2 == 2) && (idx1 == 5)) return 17;
    else if ((idx2 == 2) && (idx1 == 6)) return 18;
    else if ((idx2 == 2) && (idx1 == 7)) return 19;
    else if ((idx2 == 2) && (idx1 == 8)) return 20;

    else if ((idx2 == 3) && (idx1 == 4)) return 21;
    else if ((idx2 == 3) && (idx1 == 5)) return 22;
    else if ((idx2 == 3) && (idx1 == 6)) return 23;
    else if ((idx2 == 3) && (idx1 == 7)) return 24;
    else if ((idx2 == 3) && (idx1 == 8)) return 25;

    else if ((idx2 == 4) && (idx1 == 5)) return 26;
    else if ((idx2 == 4) && (idx1 == 6)) return 27;
    else if ((idx2 == 4) && (idx1 == 7)) return 28;
    else if ((idx2 == 4) && (idx1 == 8)) return 29;

    else if ((idx2 == 5) && (idx1 == 6)) return 30;
    else if ((idx2 == 5) && (idx1 == 7)) return 31;

    else return -1; //check if this makes sense!

  }

  int getIdxTokenFromIdxsTonesCustomMultiTone(int idx1, int idx2)
  {
    /*    if ((idx1 == 0) && (idx2 == 9)) return 0;
    else if ((idx1 == 0) && (idx2 == 8)) return 1;
    else if ((idx1 == 0) && (idx2 == 7)) return 2;
    else if ((idx1 == 0) && (idx2 == 6)) return 3;
    else if ((idx1 == 0) && (idx2 == 5)) return 4;
    else if ((idx1 == 0) && (idx2 == 4)) return 5;
    else if ((idx1 == 0) && (idx2 == 3)) return 6;
    //else if ((idx1 == 0) && (idx2 == 2)) return 7;
    else if ((idx1 == 6) && (idx2 == 8)) return 7;

    else if ((idx1 == 1) && (idx2 == 9)) return 8;
    else if ((idx1 == 1) && (idx2 == 8)) return 9;
    else if ((idx1 == 1) && (idx2 == 7)) return 10;
    else if ((idx1 == 1) && (idx2 == 6)) return 11;
    else if ((idx1 == 1) && (idx2 == 5)) return 12;
    else if ((idx1 == 1) && (idx2 == 4)) return 13;
    else if ((idx1 == 1) && (idx2 == 3)) return 14;

    else if ((idx1 == 2) && (idx2 == 9)) return 15;
    else if ((idx1 == 2) && (idx2 == 8)) return 16;
    else if ((idx1 == 2) && (idx2 == 7)) return 17;
    else if ((idx1 == 2) && (idx2 == 6)) return 18;
    else if ((idx1 == 2) && (idx2 == 5)) return 19;
    else if ((idx1 == 2) && (idx2 == 4)) return 20;

    else if ((idx1 == 3) && (idx2 == 9)) return 21;
    else if ((idx1 == 3) && (idx2 == 8)) return 22;
    else if ((idx1 == 3) && (idx2 == 7)) return 23;
    else if ((idx1 == 3) && (idx2 == 6)) return 24;
    else if ((idx1 == 3) && (idx2 == 5)) return 25;

    else if ((idx1 == 4) && (idx2 == 9)) return 26;
    else if ((idx1 == 4) && (idx2 == 8)) return 27;
    else if ((idx1 == 4) && (idx2 == 7)) return 28;
    else if ((idx1 == 4) && (idx2 == 6)) return 29;

    else if ((idx1 == 5) && (idx2 == 9)) return 30;
    else if ((idx1 == 5) && (idx2 == 8)) return 31;

    //reversed
    else if ((idx2 == 0) && (idx1 == 9)) return 0;
    else if ((idx2 == 0) && (idx1 == 8)) return 1;
    else if ((idx2 == 0) && (idx1 == 7)) return 2;
    else if ((idx2 == 0) && (idx1 == 6)) return 3;
    else if ((idx2 == 0) && (idx1 == 5)) return 4;
    else if ((idx2 == 0) && (idx1 == 4)) return 5;
    else if ((idx2 == 0) && (idx1 == 3)) return 6;
    //else if ((idx2 == 0) && (idx1 == 2)) return 7;
    else if ((idx2 == 6) && (idx1 == 8)) return 7;

    else if ((idx2 == 1) && (idx1 == 9)) return 8;
    else if ((idx2 == 1) && (idx1 == 8)) return 9;
    else if ((idx2 == 1) && (idx1 == 7)) return 10;
    else if ((idx2 == 1) && (idx1 == 6)) return 11;
    else if ((idx2 == 1) && (idx1 == 5)) return 12;
    else if ((idx2 == 1) && (idx1 == 4)) return 13;
    else if ((idx2 == 1) && (idx1 == 3)) return 14;

    else if ((idx2 == 2) && (idx1 == 9)) return 15;
    else if ((idx2 == 2) && (idx1 == 8)) return 16;
    else if ((idx2 == 2) && (idx1 == 7)) return 17;
    else if ((idx2 == 2) && (idx1 == 6)) return 18;
    else if ((idx2 == 2) && (idx1 == 5)) return 19;
    else if ((idx2 == 2) && (idx1 == 4)) return 20;

    else if ((idx2 == 3) && (idx1 == 9)) return 21;
    else if ((idx2 == 3) && (idx1 == 8)) return 22;
    else if ((idx2 == 3) && (idx1 == 7)) return 23;
    else if ((idx2 == 3) && (idx1 == 6)) return 24;
    else if ((idx2 == 3) && (idx1 == 5)) return 25;

    else if ((idx2 == 4) && (idx1 == 9)) return 26;
    else if ((idx2 == 4) && (idx1 == 8)) return 27;
    else if ((idx2 == 4) && (idx1 == 7)) return 28;
    else if ((idx2 == 4) && (idx1 == 6)) return 29;

    else if ((idx2 == 5) && (idx1 == 9)) return 30;
    else if ((idx2 == 5) && (idx1 == 8)) return 31;

    else return 0; //check if this makes sense!
    */


    if ((idx1 == 0) && (idx2 == 1)) return 0;
    else if ((idx1 == 0) && (idx2 == 2)) return 1;
    else if ((idx1 == 0) && (idx2 == 3)) return 2;
    else if ((idx1 == 0) && (idx2 == 4)) return 3;
    else if ((idx1 == 0) && (idx2 == 5)) return 4;
    else if ((idx1 == 0) && (idx2 == 6)) return 5;
    else if ((idx1 == 0) && (idx2 == 7)) return 6;
    else if ((idx1 == 0) && (idx2 == 8)) return 7;

    else if ((idx1 == 1) && (idx2 == 2)) return 8;
    else if ((idx1 == 1) && (idx2 == 3)) return 9;
    else if ((idx1 == 1) && (idx2 == 4)) return 10;
    else if ((idx1 == 1) && (idx2 == 5)) return 11;
    else if ((idx1 == 1) && (idx2 == 6)) return 12;
    else if ((idx1 == 1) && (idx2 == 7)) return 13;
    else if ((idx1 == 1) && (idx2 == 8)) return 14;

    else if ((idx1 == 2) && (idx2 == 3)) return 15;
    else if ((idx1 == 2) && (idx2 == 4)) return 16;
    else if ((idx1 == 2) && (idx2 == 5)) return 17;
    else if ((idx1 == 2) && (idx2 == 6)) return 18;
    else if ((idx1 == 2) && (idx2 == 7)) return 19;
    else if ((idx1 == 2) && (idx2 == 8)) return 20;

    else if ((idx1 == 3) && (idx2 == 4)) return 21;
    else if ((idx1 == 3) && (idx2 == 5)) return 22;
    else if ((idx1 == 3) && (idx2 == 6)) return 23;
    else if ((idx1 == 3) && (idx2 == 7)) return 24;
    else if ((idx1 == 3) && (idx2 == 8)) return 25;

    else if ((idx1 == 4) && (idx2 == 5)) return 26;
    else if ((idx1 == 4) && (idx2 == 6)) return 27;
    else if ((idx1 == 4) && (idx2 == 7)) return 28;
    else if ((idx1 == 4) && (idx2 == 8)) return 29;

    else if ((idx1 == 5) && (idx2 == 6)) return 30;
    else if ((idx1 == 5) && (idx2 == 7)) return 31;

    //reversed
    else if ((idx2 == 0) && (idx1 == 1)) return 0;
    else if ((idx2 == 0) && (idx1 == 2)) return 1;
    else if ((idx2 == 0) && (idx1 == 3)) return 2;
    else if ((idx2 == 0) && (idx1 == 4)) return 3;
    else if ((idx2 == 0) && (idx1 == 5)) return 4;
    else if ((idx2 == 0) && (idx1 == 6)) return 5;
    else if ((idx2 == 0) && (idx1 == 7)) return 6;
    else if ((idx2 == 0) && (idx1 == 8)) return 7;

    else if ((idx2 == 1) && (idx1 == 2)) return 8;
    else if ((idx2 == 1) && (idx1 == 3)) return 9;
    else if ((idx2 == 1) && (idx1 == 4)) return 10;
    else if ((idx2 == 1) && (idx1 == 5)) return 11;
    else if ((idx2 == 1) && (idx1 == 6)) return 12;
    else if ((idx2 == 1) && (idx1 == 7)) return 13;
    else if ((idx2 == 1) && (idx1 == 8)) return 14;

    else if ((idx2 == 2) && (idx1 == 3)) return 15;
    else if ((idx2 == 2) && (idx1 == 4)) return 16;
    else if ((idx2 == 2) && (idx1 == 5)) return 17;
    else if ((idx2 == 2) && (idx1 == 6)) return 18;
    else if ((idx2 == 2) && (idx1 == 7)) return 19;
    else if ((idx2 == 2) && (idx1 == 8)) return 20;

    else if ((idx2 == 3) && (idx1 == 4)) return 21;
    else if ((idx2 == 3) && (idx1 == 5)) return 22;
    else if ((idx2 == 3) && (idx1 == 6)) return 23;
    else if ((idx2 == 3) && (idx1 == 7)) return 24;
    else if ((idx2 == 3) && (idx1 == 8)) return 25;

    else if ((idx2 == 4) && (idx1 == 5)) return 26;
    else if ((idx2 == 4) && (idx1 == 6)) return 27;
    else if ((idx2 == 4) && (idx1 == 7)) return 28;
    else if ((idx2 == 4) && (idx1 == 8)) return 29;

    else if ((idx2 == 5) && (idx1 == 6)) return 30;
    else if ((idx2 == 5) && (idx1 == 7)) return 31;

    else return -1; //check if this makes sense!

  }

  //n is 0 or 1 for multitone mode
  void getFreqsFromIdxAudibleMultiTone(int idx, float samplingRate, int windowSize, float **freqs)
  {
    //float freqs[2];
    //float* freqs = new float[2];
    
    //int* idxtones = new int[2];
   /* idxtone1 = idx / 2;
    freqs[0] = getToneFromIdxAudibleMultiTone(idxtone1, samplingRate, windowSize);
    idxtone2 = (idx + numTonesAudibleMultiTone) % numTonesAudibleMultiTone;
    freqs[1] = getToneFromIdxAudibleMultiTone(idxtone2, samplingRate, windowSize);*/
    
    int* idxs = new int[2];

    getIdxsFromIdxAudibleMultiTone(idx, &idxs);
    (*freqs)[0] = getToneFromIdxAudibleMultiTone(idxs[0], samplingRate, windowSize);
    (*freqs)[1] = getToneFromIdxAudibleMultiTone(idxs[1], samplingRate, windowSize);
    
    delete[] idxs;

    return;
  }

  //n is 0 or 1 for multitone mode
  void getFreqsFromIdxNonAudibleMultiTone(int idx, float samplingRate, int windowSize, float **freqs)
  {
    //float freqs[2];
    //float* freqs = new float[2];

    //int* idxtones = new int[2];
    /* idxtone1 = idx / 2;
    freqs[0] = getToneFromIdxNonAudibleMultiTone(idxtone1, samplingRate, windowSize);
    idxtone2 = (idx + numTonesNonAudibleMultiTone) % numTonesNonAudibleMultiTone;
    freqs[1] = getToneFromIdxNonAudibleMultiTone(idxtone2, samplingRate, windowSize);*/

    int* idxs = new int[2];

    getIdxsFromIdxNonAudibleMultiTone(idx, &idxs);
    (*freqs)[0] = getToneFromIdxNonAudibleMultiTone(idxs[0], samplingRate, windowSize);
    (*freqs)[1] = getToneFromIdxNonAudibleMultiTone(idxs[1], samplingRate, windowSize);

    delete[] idxs;

    return;
  }

  //n is 0 or 1 for multitone mode
  void getFreqsFromIdxHiddenMultiTone(int idx, float samplingRate, int windowSize, float **freqs)
  {
    //float freqs[2];
    //float* freqs = new float[2];

    //int* idxtones = new int[2];
    /* idxtone1 = idx / 2;
    freqs[0] = getToneFromIdxNonAudibleMultiTone(idxtone1, samplingRate, windowSize);
    idxtone2 = (idx + numTonesNonAudibleMultiTone) % numTonesNonAudibleMultiTone;
    freqs[1] = getToneFromIdxNonAudibleMultiTone(idxtone2, samplingRate, windowSize);*/

    int* idxs = new int[2];

    getIdxsFromIdxHiddenMultiTone(idx, &idxs);
    (*freqs)[0] = getToneFromIdxHiddenMultiTone(idxs[0], samplingRate, windowSize);
    (*freqs)[1] = getToneFromIdxHiddenMultiTone(idxs[1], samplingRate, windowSize);

    delete[] idxs;

    return;
  }

  //n is 0 or 1 for multitone mode
  void getFreqsFromIdxCustomMultiTone(int idx, float samplingRate, int windowSize, float **freqs)
  {
    //float freqs[2];
    //float* freqs = new float[2];

    //int* idxtones = new int[2];
    /* idxtone1 = idx / 2;
    freqs[0] = getToneFromIdxNonAudibleMultiTone(idxtone1, samplingRate, windowSize);
    idxtone2 = (idx + numTonesNonAudibleMultiTone) % numTonesNonAudibleMultiTone;
    freqs[1] = getToneFromIdxNonAudibleMultiTone(idxtone2, samplingRate, windowSize);*/

    int* idxs = new int[2];

    getIdxsFromIdxCustomMultiTone(idx, &idxs);
    (*freqs)[0] = getToneFromIdxCustomMultiTone(idxs[0], samplingRate, windowSize);
    (*freqs)[1] = getToneFromIdxCustomMultiTone(idxs[1], samplingRate, windowSize);

    delete[] idxs;

    return;
  }

  //This function is called by getFreqsFromIdxAudibleMultiTone(...)
  //idx should be < numTonesAudibleMultiTone
  float getToneFromIdxAudibleMultiTone(int idx, float samplingRate, int windowSize)
  {
    float binToHz = samplingRate / windowSize; // 21,5332Hz for 1 bin at 44100Hz-2048ws

    int firstFreqBin = (int)(3300.f / binToHz + .5); //first token arround 3300Hz, last token arround 10080Hz

    float firstFreq = firstFreqBin * binToHz;

    int tokenDistanceInBins = (int)(750.f / binToHz + .5); //separation between tokens arround 750Hz, we need to fit 9 tones between 3.3Khz and 11Khz

    float tokenDistanceInHz = tokenDistanceInBins * binToHz;

    return firstFreq + idx*tokenDistanceInHz;
  }


  //This function is called by getFreqsFromIdxNonAudibleMultiTone(...)
  //idx should be < numTonesNonAudibleMultiTone
  float getToneFromIdxNonAudibleMultiTone(int idx, float samplingRate, int windowSize)
  {
    float binToHz = samplingRate / windowSize; // 21,5332Hz for 1 bin at 44100Hz-2048ws
        
    //int firstFreqBin = (int)(16800.f / binToHz + .5); //first token arround 16800Hz, last token arround 21447Hz
    //int firstFreqBin = (int)(17200.f / binToHz + .5); //first token arround 16800Hz, last token arround 21447Hz
    //int firstFreqBin = (int)(17800.f / binToHz + .5); //first token arround 17807.9Hz, last token arround 21425Hz
    int firstFreqBin = (int)(17800.f / binToHz + .5); //first token arround 17807.95Hz, last token arround 20714.94Hz

    float firstFreq = firstFreqBin * binToHz;

    //int tokenDistanceInBins = (int)(580.f / binToHz + .5); //separation between tokens arround 580Hz, we need to fit 9 tones between first and last bin
    //int tokenDistanceInBins = (int)(455.f / binToHz + .5); //separation between tokens arround 452Hz, we need to fit 9 tones between first and last bin
    int tokenDistanceInBins = (int)(freqOffsetForNonAudibleMultiTone*3.f / binToHz + .5); //separation between tokens arround ~387Hz, we need to fit 10 tones between first and last bin
    //int tokenDistanceInBins = (int)(210.f / binToHz + .5); //separation between token arround 210Hz
    //int tokenDistanceInBins = (int)(155.f / binToHz + .5); //separation between token arround 200Hz

    float tokenDistanceInHz = tokenDistanceInBins * binToHz;

    return firstFreq + idx*tokenDistanceInHz;
  }

  //This function is called by getFreqsFromIdxNonAudibleMultiTone(...)
  //idx should be < numTonesNonAudibleMultiTone
  float getToneFromIdxHiddenMultiTone(int idx, float samplingRate, int windowSize)
  {
    float binToHz = samplingRate / windowSize; // 21,5332Hz for 1 bin at 44100Hz-2048ws

                                               //int firstFreqBin = (int)(16800.f / binToHz + .5); //first token arround 16800Hz, last token arround 21447Hz
                                               //int firstFreqBin = (int)(17200.f / binToHz + .5); //first token arround 16800Hz, last token arround 21447Hz
                                               //int firstFreqBin = (int)(17800.f / binToHz + .5); //first token arround 17807.9Hz, last token arround 21425Hz
    int firstFreqBin = (int)(14000.f / binToHz + .5); //first token arround 18001.75Hz, last token arround 21490.13Hz

    float firstFreq = firstFreqBin * binToHz;

    //int tokenDistanceInBins = (int)(580.f / binToHz + .5); //separation between tokens arround 580Hz, we need to fit 9 tones between first and last bin
    //int tokenDistanceInBins = (int)(455.f / binToHz + .5); //separation between tokens arround 452Hz, we need to fit 9 tones between first and last bin
    int tokenDistanceInBins = (int)(freqOffsetForHiddenMultiTone*3.f / binToHz + .5); //separation between tokens arround ~387Hz, we need to fit 10 tones between first and last bin
                                                                                          //int tokenDistanceInBins = (int)(210.f / binToHz + .5); //separation between token arround 210Hz
                                                                                          //int tokenDistanceInBins = (int)(155.f / binToHz + .5); //separation between token arround 200Hz

    float tokenDistanceInHz = tokenDistanceInBins * binToHz;

    return firstFreq + idx*tokenDistanceInHz;
  }

  //This function is called by getFreqsFromIdxCustomMultiTone(...)
  //idx should be < numTonesCustomMultiTone
  float getToneFromIdxCustomMultiTone(int idx, float samplingRate, int windowSize)
  {
    float binToHz = samplingRate / windowSize; // 21,5332Hz for 1 bin at 44100Hz-2048ws

                                               //int firstFreqBin = (int)(16800.f / binToHz + .5); //first token arround 16800Hz, last token arround 21447Hz
                                               //int firstFreqBin = (int)(17200.f / binToHz + .5); //first token arround 16800Hz, last token arround 21447Hz
                                               //int firstFreqBin = (int)(17800.f / binToHz + .5); //first token arround 17807.9Hz, last token arround 21425Hz
    int firstFreqBin = (int)(Globals::freqBaseForCustomMultiTone / binToHz + .5); //first token arround 18001.75Hz, last token arround 21490.13Hz

    float firstFreq = firstFreqBin * binToHz;

    //int tokenDistanceInBins = (int)(580.f / binToHz + .5); //separation between tokens arround 580Hz, we need to fit 9 tones between first and last bin
    //int tokenDistanceInBins = (int)(455.f / binToHz + .5); //separation between tokens arround 452Hz, we need to fit 9 tones between first and last bin
    int tokenDistanceInBins = (int)(freqOffsetForCustomMultiTone*3.f / binToHz + .5); //separation between tokens arround ~387Hz, we need to fit 10 tones between first and last bin
                                                                                      //int tokenDistanceInBins = (int)(210.f / binToHz + .5); //separation between token arround 210Hz
                                                                                      //int tokenDistanceInBins = (int)(155.f / binToHz + .5); //separation between token arround 200Hz

    float tokenDistanceInHz = tokenDistanceInBins * binToHz;

    return firstFreq + idx*tokenDistanceInHz;
  }

  float getLoudnessFromIdx(int idx)
  {
//    float maxAtt = 0.5f;
//    float loudness = 1.f - maxAtt + (((float)idx/(float)(numFreqs-1)) * maxAtt);
//    return loudness;
    
    // apply a log scale attenaution for high freqs
    float maxAttdB = -6.f;
    float loudnessdB = powf(10.f, maxAttdB * (1 - ((float)idx/(float)(numTokensAudible-1))) / 20.f );
    
    return loudnessdB;
  }

  void getLoudnessAudibleMultiToneFromIdx(int idx, float** freqsLoudness)
  {
    //    float maxAtt = 0.5f;
    //    float loudness = 1.f - maxAtt + (((float)idx/(float)(numFreqs-1)) * maxAtt);
    //    return loudness;

    //float loudnessdB[2];
    //float* loudnessdB = new float[2];

    int* idxs = new int[2];

    getIdxsFromIdxAudibleMultiTone(idx, &idxs);
    //(*freqs)[0] = getToneFromIdxAudibleMultiTone(idxs[0], samplingRate, windowSize);
    //(*freqs)[1] = getToneFromIdxAudibleMultiTone(idxs[1], samplingRate, windowSize);
    
    // apply a log scale attenaution for high freqs
    float maxAttdB = -6.f;
    (*freqsLoudness)[0] = powf(10.f, maxAttdB * (1 - ((float)idxs[0] / (float)(numTonesAudibleMultiTone - 1))) / 20.f);
    (*freqsLoudness)[1] = powf(10.f, maxAttdB * (1 - ((float)idxs[1] / (float)(numTonesAudibleMultiTone - 1))) / 20.f);

    delete[] idxs;

    (*freqsLoudness)[0] = 1.f;
    (*freqsLoudness)[1] = 1.f;

    return;
  }

  void getLoudnessNonAudibleMultiToneFromIdx(int idx, float** freqsLoudness)
  {
    //    float maxAtt = 0.5f;
    //    float loudness = 1.f - maxAtt + (((float)idx/(float)(numFreqs-1)) * maxAtt);
    //    return loudness;

    //float loudnessdB[2];
    //float* loudnessdB = new float[2];

    int* idxs = new int[2];

    getIdxsFromIdxNonAudibleMultiTone(idx, &idxs);
    //(*freqs)[0] = getToneFromIdxNonAudibleMultiTone(idxs[0], samplingRate, windowSize);
    //(*freqs)[1] = getToneFromIdxNonAudibleMultiTone(idxs[1], samplingRate, windowSize);

    // apply a log scale attenaution for high freqs
    //float maxAttdB = -6.f;
    //(*freqsLoudness)[0] = powf(10.f, maxAttdB * (1 - ((float)idxs[0] / (float)(numTonesNonAudibleMultiTone - 1))) / 20.f);
    //(*freqsLoudness)[1] = powf(10.f, maxAttdB * (1 - ((float)idxs[1] / (float)(numTonesNonAudibleMultiTone - 1))) / 20.f);

    //Apply linear attenuation to lower freqs
    (*freqsLoudness)[0] = 0.85f + 0.15f * (float)idxs[0] / (float)(numTonesNonAudibleMultiTone - 1);
    (*freqsLoudness)[1] = 0.85f + 0.15f * (float)idxs[1] / (float)(numTonesNonAudibleMultiTone - 1);


    delete[] idxs;

    //(*freqsLoudness)[0] = 1.f;
    //(*freqsLoudness)[1] = 1.f;

    return;
  }


  void getLoudnessHiddenMultiToneFromIdx(int idx, float** freqsLoudness)
  {
    //    float maxAtt = 0.5f;
    //    float loudness = 1.f - maxAtt + (((float)idx/(float)(numFreqs-1)) * maxAtt);
    //    return loudness;

    //float loudnessdB[2];
    //float* loudnessdB = new float[2];

    int* idxs = new int[2];

    getIdxsFromIdxHiddenMultiTone(idx, &idxs);
    //(*freqs)[0] = getToneFromIdxNonAudibleMultiTone(idxs[0], samplingRate, windowSize);
    //(*freqs)[1] = getToneFromIdxNonAudibleMultiTone(idxs[1], samplingRate, windowSize);

    // apply a log scale attenaution for high freqs
    //float maxAttdB = -6.f;
    //(*freqsLoudness)[0] = powf(10.f, maxAttdB * (1 - ((float)idxs[0] / (float)(numTonesNonAudibleMultiTone - 1))) / 20.f);
    //(*freqsLoudness)[1] = powf(10.f, maxAttdB * (1 - ((float)idxs[1] / (float)(numTonesNonAudibleMultiTone - 1))) / 20.f);

    //Apply linear attenuation to lower freqs
    (*freqsLoudness)[0] = 0.85f + 0.15f * (float)idxs[0] / (float)(numTonesHiddenMultiTone - 1);
    (*freqsLoudness)[1] = 0.85f + 0.15f * (float)idxs[1] / (float)(numTonesHiddenMultiTone - 1);


    delete[] idxs;

    //(*freqsLoudness)[0] = 1.f;
    //(*freqsLoudness)[1] = 1.f;

    return;
  }

  void getLoudnessCustomMultiToneFromIdx(int idx, float** freqsLoudness)
  {
    //    float maxAtt = 0.5f;
    //    float loudness = 1.f - maxAtt + (((float)idx/(float)(numFreqs-1)) * maxAtt);
    //    return loudness;

    //float loudnessdB[2];
    //float* loudnessdB = new float[2];

    int* idxs = new int[2];

    getIdxsFromIdxCustomMultiTone(idx, &idxs);
    //(*freqs)[0] = getToneFromIdxNonAudibleMultiTone(idxs[0], samplingRate, windowSize);
    //(*freqs)[1] = getToneFromIdxNonAudibleMultiTone(idxs[1], samplingRate, windowSize);

    // apply a log scale attenaution for high freqs
    //float maxAttdB = -6.f;
    //(*freqsLoudness)[0] = powf(10.f, maxAttdB * (1 - ((float)idxs[0] / (float)(numTonesNonAudibleMultiTone - 1))) / 20.f);
    //(*freqsLoudness)[1] = powf(10.f, maxAttdB * (1 - ((float)idxs[1] / (float)(numTonesNonAudibleMultiTone - 1))) / 20.f);

    //Apply linear attenuation to lower freqs
    (*freqsLoudness)[0] = 0.85f + 0.15f * (float)idxs[0] / (float)(numTonesCustomMultiTone - 1);
    (*freqsLoudness)[1] = 0.85f + 0.15f * (float)idxs[1] / (float)(numTonesCustomMultiTone - 1);


    delete[] idxs;

    //(*freqsLoudness)[0] = 1.f;
    //(*freqsLoudness)[1] = 1.f;

    return;
  }

  float getMusicalNoteFromIdx(int idx)
  {    
    float firstFreq = 440.f;

    return firstFreq * pow(2.f,(float)idx/12.f);
  }

  float maxValue(float *myArray, int size)
  {   
    //assert(myArray && size);
    int i;
    float maxValue = myArray[0];

    for (i=1;i<size;i++)
      if (myArray[i]>maxValue)
        maxValue = myArray[i];
    return maxValue;
  }

  int maxValue(int *myArray, int size)
  {   
    //assert(myArray && size);
    int i;
    int maxValue = myArray[0];

    for (i=1;i<size;i++)
      if (myArray[i]>maxValue)
        maxValue = myArray[i];
    return maxValue;
  }

  float secondValue(float *myArray, int size)
  {   
    //assert(myArray && size);
    int i;
    int maxValIdx = maxValueIdx(myArray, size);
    
    float secondValue = myArray[0];
    if (maxValIdx==0)
      secondValue = myArray[1];

    for (i=0;i<size;i++)
      if (i != maxValIdx)
        if (myArray[i] > secondValue)
          secondValue = myArray[i];

    return secondValue;
  }

  int maxValueIdx(float *myArray, int size)
  {   
    //assert(myArray && size);
    int idx = 0;
    int i;
    float maxValue = myArray[0];

    for (i=1;i<size;i++)
      if (myArray[i]>maxValue)
      {
        idx = i;
        maxValue = myArray[i];
      }
    
    return idx;
  }

  int maxValueIdx(int *myArray, int size)
  {   
    //assert(myArray && size);
    int idx = 0;
    int i;
    int maxValue = myArray[0];

    for (i=1;i<size;i++)
      if (myArray[i]>maxValue)
      {
        idx = i;
        maxValue = myArray[i];
      }
    
    return idx;
  }

  int secondValueIdx(float *myArray, int size)
  {   
    int i;
    int secondidx = 0;
    int maxValIdx = maxValueIdx(myArray, size);
    
    float secondValue = myArray[0];
    if (maxValIdx == 0)
    {
      secondValue = myArray[1];
      secondidx = 1;
    }

    for (i=0;i<size;i++)
      if (i != maxValIdx)
        if (myArray[i] > secondValue)
        {
          secondidx = i;
          secondValue = myArray[i];
        }

    return secondidx;
  }
  

  int secondValueIdx(int *myArray, int size)
  {
    int i;
    int secondidx = 0;
    int maxValIdx = maxValueIdx(myArray, size);

    int secondValue = myArray[0];
    if (maxValIdx == 0)
    {
      secondValue = myArray[1];
      secondidx = 1;
    }

    for (i = 0; i<size; i++)
      if (i != maxValIdx)
        if (myArray[i] > secondValue)
        {
          secondidx = i;
          secondValue = myArray[i];
        }

    return secondidx;
  }


  float sum(float *data, int size)
  {
    float sum=0.0;
    for(int i=0;i<size;++i)
      sum+=data[i];
    return sum;
  }

  float square_sum(float *data, int size)
  {
    float square_sum = 0.0;
    for (int i = 0; i<size; ++i)
      square_sum += powf(data[i], 2.f);
    return square_sum;
  }

  float mean(float *data, int size)
  {
    float mean=0.0;
    for(int i=0;i<size;++i)
      mean+=data[i];
    return mean/size;
  }

  float standard_deviation(float *data, int size)
  {
    float mean=0.0, sum_deviation=0.0;
    int i;
    for(i=0;i<size;++i)
      mean+=data[i];
    mean=mean/size;
    for(i=0;i<size;++i)
      sum_deviation+=(data[i]-mean)*(data[i]-mean);
    return sqrt(sum_deviation/size);           
  }

  //standard deviation when mean is provided
  float standard_deviation(float *data, float mean, int size)
  {
    float sum_deviation=0.0;
    for(int i=0;i<size;++i)
      sum_deviation+=(data[i]-mean)*(data[i]-mean);
    return sqrt(sum_deviation/size);           
  }
  
}