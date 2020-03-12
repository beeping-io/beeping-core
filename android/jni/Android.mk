LOCAL_PATH := $(call my-dir)
ROOT_PATH := $(LOCAL_PATH)

include $(call all-subdir-makefiles)
include $(CLEAR_VARS)

LOCAL_PATH = $(ROOT_PATH)
LOCAL_CFLAGS := -Wall -Wextra

LOCAL_CFLAGS := -ffast-math -O3 -funroll-loops -fno-strict-aliasing -mfpu=vfp -mfloat-abi=softfp

LOCAL_CPP_FEATURES += exceptions

LOCAL_CFLAGS    := -DNDEBUG -DLINUX -DDISABLE_FLOATING_POINT_PRECISION_WARNING -DANDROID

LOCAL_MODULE    := beepingcore

LOCAL_C_INCLUDES += ../../BeepingCoreLib \
                    ../../BeepingCoreLib/src/ \
                    ../../BeepingCoreLib/include \
                    ../../BeepingCoreLib/BeepingCoreLib/src \
                    ../../BeepingCoreLib/BeepingCoreLib/src/fftooura
					
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib  
# for native audio
LOCAL_LDLIBS += -lOpenSLES
# for logging
LOCAL_LDLIBS += -ldl -llog

LOCAL_STATIC_LIBRARIES := 
LOCAL_SRC_FILES := src/AndroidBeepingOpenSL.c \
          src/opensl_io.c \
          src/BeepingCore.cpp \
          ../../BeepingCoreLib/src/BeepingCoreLib_api.cpp \
          ../../BeepingCoreLib/src/Encoder.cpp \
          ../../BeepingCoreLib/src/Decoder.cpp \
          ../../BeepingCoreLib/src/EncoderAudible.cpp \
          ../../BeepingCoreLib/src/DecoderAudible.cpp \
          ../../BeepingCoreLib/src/EncoderNonAudible.cpp \
          ../../BeepingCoreLib/src/DecoderNonAudible.cpp \
          ../../BeepingCoreLib/src/EncoderAudibleMultiTone.cpp \
          ../../BeepingCoreLib/src/DecoderAudibleMultiTone.cpp \
          ../../BeepingCoreLib/src/EncoderHiddenMultiTone.cpp \
          ../../BeepingCoreLib/src/DecoderHiddenMultiTone.cpp \
          ../../BeepingCoreLib/src/EncoderNonAudibleMultiTone.cpp \
          ../../BeepingCoreLib/src/DecoderNonAudibleMultiTone.cpp \
          ../../BeepingCoreLib/src/DecoderAllMultiTone.cpp \
          ../../BeepingCoreLib/src/EncoderCustomMultiTone.cpp \
          ../../BeepingCoreLib/src/DecoderCustomMultiTone.cpp \
          ../../BeepingCoreLib/src/Globals.cpp \
          ../../BeepingCoreLib/src/SpectralAnalysis.cpp \
          ../../BeepingCoreLib/src/ReedSolomon.cpp

include $(BUILD_SHARED_LIBRARY)
