#ifndef __BEEPINGCORELIB__
#define __BEEPINGCORELIB__

#include <jni.h>

#define BC_TOKEN_START     0
#define BC_TOKEN_END_OK    1
#define BC_TOKEN_END_BAD   2
#define BC_END_PLAY        3

#ifdef __cplusplus
extern "C" {
#endif 
  
  jint beeping_callback(jint);  
  
  JNIEXPORT jlong JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_init(JNIEnv *env, jobject obj);
  
  JNIEXPORT void JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_dealloc(JNIEnv *env, jobject obj, jlong beepingObject);
  
  JNIEXPORT void JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_start(JNIEnv *env, jobject obj, jlong beepingObject);
  
  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_configure(JNIEnv *env, jobject obj, jint mode, jlong beepingObject);
      
  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_startBeepingListen(JNIEnv *env, jobject obj, jlong beepingObject);

  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_stopBeepingListen(JNIEnv *env, jobject obj, jlong beepingObject);
  
  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_playBeeping(JNIEnv *env, jobject obj, jstring code, jlong beepingObject);

  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodedString(JNIEnv *env, jobject obj, jcharArray stringDecoded, jlong beepingObject);

  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getVersionCoreLib(JNIEnv *env, jobject obj, jcharArray stringVersionInfo, jlong beepingObject);
  
  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_setAudioSignature(JNIEnv *env, jobject obj, jint size, jfloatArray audioBuffer, jlong beepingObject);

  JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getConfidence(JNIEnv *env, jobject obj, jlong beepingObject);

  JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getConfidenceError(JNIEnv *env, jobject obj, jlong beepingObject);

  JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getConfidenceNoise(JNIEnv *env, jobject obj, jlong beepingObject);

  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodedMode(JNIEnv *env, jobject obj, jlong beepingObject);


  JNIEXPORT jint JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_setCustomBaseFreq(JNIEnv *env, jobject obj, jfloat baseFreq, jint beepsSeparation, jlong beepingObject);

  JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getReceivedBeepsVolume(JNIEnv *env, jobject obj, jlong beepingObject);

  JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodingBeginFreq(JNIEnv *env, jobject obj, jlong beepingObject);

  JNIEXPORT jfloat JNICALL Java_com_beeping_AndroidBeepingCore_BeepingCoreJNI_getDecodingEndFreq(JNIEnv *env, jobject obj, jlong beepingObject);


#ifdef __cplusplus
}
#endif


#endif //__BEEPINGCORELIB__
