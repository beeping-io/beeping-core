#include "BeepingCore.h"
#include "BeepingCoreLib_api.h"

#include "AndroidBeepingOpenSL.h"

#include <android/log.h>

#include <iostream>
#include <ctime>

#include <time.h>

#include <unistd.h>

/* return current time in milliseconds */
static double now_ms(void) {

    struct timespec res;
    clock_gettime(CLOCK_REALTIME, &res);
    return 1000.0 * res.tv_sec + (double) res.tv_nsec / 1e6;
}

//Define these in Android.mk
//#define _ANDROID_LOG_
//#define _ANDROID_LOG_TIME_COMMENT

//Begin Global vars
JavaVM* g_vm = NULL;
jobject g_obj = NULL;
jmethodID g_mid = NULL; 

JNIEnv *g_env = NULL;

void* g_BeepingCore = NULL;
int g_playEncode = 0;
int g_Decoding = 0;

char* g_StringDecoded;
int g_SizeStringDecoded;

char* g_StringVersionInfo;
//End Global vars


//------------------------------------
// internal objects
//------------------------------------

#ifdef __cplusplus
extern "C" {
#endif 

int beeping_callback(int value)
{
  if ( ((g_env!=NULL) && (g_obj!=NULL)) && (g_mid!=NULL) )
  {
    g_env->CallVoidMethod(g_obj, g_mid, value);
    return 0;
  }
  else
  {
    //should we send a message to the client?
    __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Callback not configured");
    return -1;
  }
    
}

JNIEXPORT jlong JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_init(JNIEnv *env, jobject obj)
{
  //jclass javaClassRef = env->FindClass("com/beeping/AndroidBeepingCore/BeepingCoreJNI");
  
  //jmethodID javaMethodRef = env->GetMethodID(javaClassRef, "BeepingCallback", "(I)V");
  //jmethodID javaMethodRef = env->GetStaticMethodID(javaClassRef, "MyCallback", "(I)V");
  
  //jint value = 10;

  //g_env = env;
  //g_obj = obj;
  //g_mid = javaMethodRef;
  
  //env->CallVoidMethod(javaClassRef, javaMethodRef, depth);
  //env->CallVoidMethod(obj, javaMethodRef, depth);
  //g_env->CallVoidMethod(g_obj, g_mid, depth);
  
  //beeping_callback(depth);
    
  
  //start_audio();
  
  g_StringDecoded = new char[30];
  
  g_StringVersionInfo = new char[100];

  void* ptr = BEEPING_Create();
  g_BeepingCore = ptr;
  return (long)ptr;
  //return 0;
}

JNIEXPORT void JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_dealloc(JNIEnv *env, jobject obj, jlong beepingObject)
{ 
  g_Decoding = 0;
  stop_audio();
  
  delete [] g_StringDecoded;

  delete [] g_StringVersionInfo;
  
  return BEEPING_Destroy((void*)beepingObject);
}

JNIEXPORT void JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_start(JNIEnv *env, jobject obj, jlong beepingObject)
{
  g_Decoding = 0;
  g_playEncode = 0;
  
  jclass javaClassRef = env->FindClass("com/beeping/AndroidBeepingCore/BeepingCoreJNI");
  
  jmethodID javaMethodRef = env->GetMethodID(javaClassRef, "BeepingCallback", "(I)V");
  //jmethodID javaMethodRef = env->GetStaticMethodID(javaClassRef, "MyCallback", "(I)V");

  g_env = env;
  g_obj = obj;
  g_mid = javaMethodRef;
  
  //stop_audio();
  start_audio();
  return;
}

JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_configure(JNIEnv *env, jobject obj, jint mode, jlong beepingObject)
{
  //stop_audio();
  g_Decoding = 0;
  g_playEncode = 0;
  
  //sleep(1); //in seconds
  usleep(500000); //in microseconds 500000 = 0.5 seconds
  jfloat samplingRate = 44100.f;
  //jint bufferSize = 512;
  jint bufferSize = 64; //same value as #define VECSAMPS_MONO 64 in AndroidBepingOpenSL.c
  int ret = BEEPING_Configure(mode, samplingRate,bufferSize,(void*)beepingObject);
  
/*  jclass javaClassRef = env->FindClass("com/beeping/AndroidBeepingCore/BeepingCoreJNI");
  
  jmethodID javaMethodRef = env->GetMethodID(javaClassRef, "BeepingCallback", "(I)V");
  //jmethodID javaMethodRef = env->GetStaticMethodID(javaClassRef, "MyCallback", "(I)V");

  g_env = env;
  g_obj = obj;
  g_mid = javaMethodRef;*/
  
  
  
  
  //env->CallVoidMethod(javaClassRef, javaMethodRef, depth);
  //env->CallVoidMethod(obj, javaMethodRef, depth);
  //g_env->CallVoidMethod(g_obj, g_mid, depth);
  
  //beeping_callback(depth);
  
  //start_audio();
 
  return 0;
  
/*  jclass javaClassRef = env->FindClass("com/beeping/AndroidBeepingCore/BeepingCoreJNI");
  
  jmethodID javaMethodRef = env->GetMethodID(javaClassRef, "MyCallback", "(I)V");
  //jmethodID javaMethodRef = env->GetStaticMethodID(javaClassRef, "MyCallback", "(I)V");
  
  jint depth = 10;

  g_env = env;
  g_obj = obj;
  g_mid = javaMethodRef;
  
  //env->CallVoidMethod(javaClassRef, javaMethodRef, depth);
  //env->CallVoidMethod(obj, javaMethodRef, depth);
  //g_env->CallVoidMethod(g_obj, g_mid, depth);

  beeping_callback(depth);
  

  
  return 0;*/
    
  
  //env->GetJavaVM(&g_vm);
  // convert local to global reference 
                // (local will die after this method call)
		//g_obj = env->NewGlobalRef(obj);

		// save refs for callback
		//jclass g_clazz = env->GetObjectClass(g_obj);
/*    jclass g_clazz = env->FindClass("com/beeping/AndroidBeepingCore/BeepingCore");
		if (g_clazz == NULL) {
			std::cout << "Failed to find class" << std::endl;
		}

		g_mid = env->GetMethodID(g_clazz, "MyCallback", "(I)V");
		if (g_mid == NULL) {
			std::cout << "Unable to get method ref" << std::endl;
		}*/
  


  
/*    JNIEnv * g_env;
	// double check it's all ok
	int getEnvStat = g_vm->GetEnv((void **)&g_env, JNI_VERSION_1_6);
	if (getEnvStat == JNI_EDETACHED)
  {
	__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Getenv not attached");
		if (g_vm->AttachCurrentThread(&g_env, NULL) != 0) {
			__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Failed to attach");
		}
	} else if (getEnvStat == JNI_OK) {
		//
	} else if (getEnvStat == JNI_EVERSION) {
		__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "GetEnv: version not supported");
	}

  jclass g_clazz = g_env->FindClass("com/beeping/AndroidBeepingCore/BeepingCore");
		if (g_clazz == NULL) {
			__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Failed to find class");
		}

		g_mid = g_env->GetMethodID(g_clazz, "MyCallback", "(I)V");
		if (g_mid == NULL) {
			__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Unable to get method ref");
		}
  
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Calling Void Method");
	g_env->CallVoidMethod(g_clazz, g_mid, 10);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "Void Method called");

	if (g_env->ExceptionCheck()) {
		g_env->ExceptionDescribe();
	}

	g_vm->DetachCurrentThread();
    
    return 0;
  
  */
  
  //jclass cls = env->GetObjectClass(obj);
  //jclass javaClassRef = (jclass) env->NewGlobalRef(cls);
  
  //jclass javaClassRef = env->FindClass("com/VoctroLabs/AndroidBeepingCoreLibTest/MainActivity");
  //jmethodID javaMethodRef  = env->GetMethodID(javaClassRef, "MyCallback", "(I)V");
  //jmethodID javaMethodRef  = env->GetMethodID(obj, "MyCallback", "(I)V");

/*  
  jclass javaClassRef = env->FindClass("com/beeping/AndroidBeepingCore/BeepingCore");
  
  jmethodID javaMethodRef = env->GetMethodID(javaClassRef, "MyCallback", "(I)V");
  //jmethodID javaMethodRef = env->GetStaticMethodID(javaClassRef, "MyCallback", "(I)V");
  
  jint depth = 10;
  
  env->CallVoidMethod(javaClassRef, javaMethodRef, depth);
  //env->CallStaticVoidMethod(javaClassRef, javaMethodRef, depth);
  */
  
  
  /*jclass activityClass = env->GetObjectClass(obj);
  jmethodID javaMethodRef = env->GetMethodID(activityClass, "MyCallback", "(I)V");
  jint depth = 10;
  //printf("In C, depth = %d, about to enter Java\n", depth);
  env->CallVoidMethod(obj, javaMethodRef, depth);*/
  
/*  
 __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "0");//Or ANDROID_LOG_INFO, ...
  env->GetJavaVM(&javaVM);
__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "1");//Or ANDROID_LOG_INFO, ...
  jclass cls = env->GetObjectClass(obj);
__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "2");//Or ANDROID_LOG_INFO, ...  
  //activityClass = (jclass) env->NewGlobalRef(cls);
  //activityClass = (jclass) env->FindClass("com/VoctroLabs/AndroidBeepingCoreLibTest/MainActivity");
  activityClass = (jclass) env->FindClass("com/VoctroLabs/BeepingCoreInterface/BeepingCoreInterfaceJNI");
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "3");//Or ANDROID_LOG_INFO, ...
  activityObj = env->NewGlobalRef(obj);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "4");//Or ANDROID_LOG_INFO, ...
  //JNIEnv *env;
  javaVM->AttachCurrentThread(&env, NULL);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "5");//Or ANDROID_LOG_INFO, ...
  jmethodID method = env->GetMethodID(activityClass, "MyCallback", "(I)V");
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "6");//Or ANDROID_LOG_INFO, ...
  env->CallVoidMethod(activityObj, method);
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "7");//Or ANDROID_LOG_INFO, ...
  */
/*  if (javaMethodRef  == 0)
  {
#ifdef _ANDROID_LOG_
  __android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "MyCallback method not found: ");//Or ANDROID_LOG_INFO, ...
  
#endif

    return -1;
  }
  
  jint depth = 10;
  printf("In C, depth = %d, about to enter Java\n", depth);
  env->CallVoidMethod(obj, javaMethodRef, depth);
  printf("In C, depth = %d, back from Java\n", depth);
  */
}

JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_startBeepingListen(JNIEnv *env, jobject obj, jlong beepingObject)
{
  g_Decoding = 1;

  return 0;
}

JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_stopBeepingListen(JNIEnv *env, jobject obj, jlong beepingObject)
{
  g_Decoding = 0;

  return 0;
}
  
JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_playBeeping(JNIEnv *env, jobject obj, jstring stringToEncode, jlong beepingObject)
{
  const char *nativeStringToEncode = env->GetStringUTFChars(stringToEncode, JNI_FALSE);
  
  //__android_log_write(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "playBeeping native ");
  int type = 0; //0: no melody, 1: random melody
  int sizeSamples = BEEPING_EncodeDataToAudioBuffer(nativeStringToEncode,9,type,NULL,0,(void*)beepingObject);
  env->ReleaseStringUTFChars(stringToEncode, nativeStringToEncode);
  
  env->DeleteLocalRef(stringToEncode); //is it necessary?
  
  g_playEncode = 1;

  return 0;
}


JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodedString(JNIEnv *env, jobject obj, jcharArray stringDecoded, jlong beepingObject)
{
  //__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "getDecodedString start");
  jchar* pStringDecoded = (jchar*)env->GetPrimitiveArrayCritical(stringDecoded, 0); //to get access to the java allocated String to modify elements from native side  
    
  int abs_sizeCharsDecoded = (g_SizeStringDecoded >= 0) ? g_SizeStringDecoded : -g_SizeStringDecoded;
    
  for (int i=0;i<abs_sizeCharsDecoded;i++)
    pStringDecoded[i] = (jchar)g_StringDecoded[i];

  env->ReleasePrimitiveArrayCritical(stringDecoded, pStringDecoded, 0);
  env->DeleteLocalRef(stringDecoded); //is it necessary?
  
  return g_SizeStringDecoded;
}

JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getVersionCoreLib(JNIEnv *env, jobject obj, jcharArray stringVersionInfo, jlong beepingObject)
{
  //__android_log_print(ANDROID_LOG_INFO, "BeepingCoreLibInfo", "getDecodedString start");
  jchar* pStringVersionInfo = (jchar*)env->GetPrimitiveArrayCritical(stringVersionInfo, 0); //to get access to the java allocated String to modify elements from native side  
    
  //BEEPING_GetVersion();
  char* pStringVersionInfoTmp = new char[100];

  int sizeCharsVersionInfo = BEEPING_GetVersionInfo(pStringVersionInfoTmp);

  //jCharArray tmpStringVersionInfo = BEEPING_GetVersion();

  for (int i=0;i<sizeCharsVersionInfo;i++)
    pStringVersionInfo[i] = (jchar)pStringVersionInfoTmp[i];

  delete [] pStringVersionInfoTmp;

  env->ReleasePrimitiveArrayCritical(stringVersionInfo, pStringVersionInfo, 0);
  env->DeleteLocalRef(stringVersionInfo); //is it necessary?
  
  return sizeCharsVersionInfo;

}

JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_setAudioSignature(JNIEnv *env, jobject obj, jint size, jfloatArray audioBuffer, jlong beepingObject)
{
  int ret = 0;
  if ((audioBuffer == NULL) || (size<=0))
  {
    ret = BEEPING_SetAudioSignature(0, NULL, (void*)beepingObject);
  }
  else
  {
    float* cAudioBuffer = (float *)env->GetFloatArrayElements(audioBuffer,0); //array copied
    ret = BEEPING_SetAudioSignature(size, cAudioBuffer, (void*)beepingObject);
    env->ReleaseFloatArrayElements(audioBuffer, cAudioBuffer, 0); 
  }
  return ret;
}

JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getConfidence(JNIEnv *env, jobject obj, jlong beepingObject)
{
  return BEEPING_GetConfidence((void*)beepingObject);
}

JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getConfidenceError(JNIEnv *env, jobject obj, jlong beepingObject)
{

  return BEEPING_GetConfidenceError((void*)beepingObject);
}

JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getConfidenceNoise(JNIEnv *env, jobject obj, jlong beepingObject)
{

  return BEEPING_GetConfidenceNoise((void*)beepingObject);
}

JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodedMode(JNIEnv *env, jobject obj, jlong beepingObject)
{

  return BEEPING_GetDecodedMode((void*)beepingObject);
}


JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_setCustomBaseFreq(JNIEnv *env, jobject obj, jfloat baseFreq, jint beepsSeparation, jlong beepingObject)
{
  return BEEPING_SetCustomBaseFreq(baseFreq, beepsSeparation, (void*)beepingObject);
}

JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getReceivedBeepsVolume(JNIEnv *env, jobject obj, jlong beepingObject)
{
  return BEEPING_GetReceivedBeepsVolume((void*)beepingObject);
}
 
JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodingBeginFreq(JNIEnv *env, jobject obj, jlong beepingObject)
{
  return BEEPING_GetDecodingBeginFreq((void*)beepingObject);
}

JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodingEndFreq(JNIEnv *env, jobject obj, jlong beepingObject)
{
  return BEEPING_GetDecodingEndFreq((void*)beepingObject);
}



#ifdef __cplusplus
}
#endif 