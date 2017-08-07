LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := vokaturi-lib
LOCAL_SRC_FILES := vokaturi-lib.cpp Vokaturi9.c WavFile.h Vokaturi.h Pitch.h Sound.h Sound_and_Pitch.h Thing.h Cues9.h Emotions.h fft_c.h Network9-100-20.h Sound_and_Spectrum.h Sound_save.h Spectrum.h VokaturiQuality.h VokaturiException.h
LOCAL_LDLIBS += -lm -landroid -llog

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

#traverse all the directory and subdirectory
define walk
  $(wildcard $(1)) $(foreach e, $(wildcard $(1)/*), $(call walk, $(e)))
endef

#-landroid
#find all the file recursively under jni/
ALLFILES = $(call walk, $(LOCAL_PATH))
FILE_LIST := $(filter %.cpp, $(ALLFILES))

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_SHARED_LIBRARY)