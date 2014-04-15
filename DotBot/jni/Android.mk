LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
  
APP_CFLAGS := -DNDK_DEBUG=1

LOCAL_LDLIBS := -llog
 
LOCAL_MODULE := dotbot
LOCAL_SRC_FILES := screenreader.c screenshot.c
LOCAL_SRC_FILES += dotssolver.c
 
include $(BUILD_SHARED_LIBRARY)
