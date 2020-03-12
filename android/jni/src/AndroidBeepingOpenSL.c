/*
AndroidBeepingOpenSL.c:
*/

#ifdef _ANDROID_LOG_
  #include <android/log.h>
#endif

#include "BeepingCore.h"

#include "opensl_io.h"


#define BUFFERFRAMES 1024
#define VECSAMPS_MONO 64
//#define VECSAMPS_MONO 256
#define VECSAMPS_STEREO 128
//#define VECSAMPS_STEREO 512

#define SR 44100 

extern void* g_BeepingCore;
extern int g_playEncode;
extern int g_Decoding;

extern char* g_StringDecoded;
extern int g_SizeStringDecoded;


static int on;

void start_audio()
{
  OPENSL_STREAM  *p;
  int samps, i, j;
  int nChannelsIn = 1;
  int nChannelsOut = 2;
  
  float audioBuffer[BUFFERFRAMES];

#ifdef _ANDROID_LOG_  
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Starting Audio");
#endif //_ANDROID_LOG_
  
  float inbuffer[VECSAMPS_MONO];
  float outbuffer[VECSAMPS_STEREO];
  
  p = android_OpenAudioDevice(SR,nChannelsIn,nChannelsOut,BUFFERFRAMES); 
  
  if (p == NULL)
    return;
  
  on = 1;

#ifdef _ANDROID_LOG_  
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Audio started");
#endif //_ANDROID_LOG_

  while (on)
  {
    samps = android_AudioIn(p,inbuffer,VECSAMPS_MONO);
    
    if (g_Decoding == 1)
    {
      //__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Decoding...");
    
      int ret = BEEPING_DecodeAudioBuffer(inbuffer, samps, g_BeepingCore);
      
      if (ret == -2)
      {
        beeping_callback(BC_TOKEN_START);
#ifdef _ANDROID_LOG_
        __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "BEGIN TOKEN FOUND!");
#endif //_ANDROID_LOG_
      }
      else if (ret >= 0)
      {
        //beeping_callback(BC_);
#ifdef _ANDROID_LOG_
        __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Token Found! %d", ret);
#endif //_ANDROID_LOG_
      }
      else if (ret == -3)
      {        
        g_SizeStringDecoded = BEEPING_GetDecodedData(g_StringDecoded, g_BeepingCore);
        int mDecodedOK;
        if (g_SizeStringDecoded > 0)
          mDecodedOK = 1;
        else
          mDecodedOK = -1;
                          
        if (mDecodedOK > 0)
        {
          beeping_callback(BC_TOKEN_END_OK);
#ifdef _ANDROID_LOG_          
          __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "[succesful] END DECODING! %s", g_StringDecoded);
#endif //_ANDROID_LOG_          
        }
        else
        {
          beeping_callback(BC_TOKEN_END_BAD);
#ifdef _ANDROID_LOG_          
          __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "[error] END DECODING! %s", g_StringDecoded);
#endif //_ANDROID_LOG_          
        }
      }
      else
      {
        //No data found in this buffer
      }
    }

    if (g_playEncode == 1)
    {
      int sizeSamplesRead = BEEPING_GetEncodedAudioBuffer(audioBuffer, g_BeepingCore);
      //__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "SizeSamplesRead: %d", sizeSamplesRead);
      if (sizeSamplesRead > 0)
      {       
        for(i=0,j=0; i<sizeSamplesRead; i++,j+=2)
        {
          outbuffer[j] = outbuffer[j+1] = audioBuffer[i];
        }
      }
      else
      {
        beeping_callback(BC_END_PLAY);
#ifdef _ANDROID_LOG_
        __android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "stop playing"); 
#endif //_ANDROID_LOG_        
        g_playEncode = 0;
      }
    }
    else
    {
      for(i=0,j=0; i<samps; i++,j+=2)
      {
        outbuffer[j] = outbuffer[j+1] = 0;
      }
    }
   
    android_AudioOut(p,outbuffer,samps*nChannelsOut);
   
  }  
  android_CloseAudioDevice(p);
}

void stop_audio()
{
  on = 0;
  usleep(250000); //wait 250 ms
}
