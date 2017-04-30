LOCAL_PATH := $(call my-dir)

CVROOT := /home/adesh/OpenCV-android-sdk/sdk/native/jni

include $(CLEAR_VARS)
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=STATIC
include $(CVROOT)/OpenCV.mk



LOCAL_MODULE:=Project_Library
LOCAL_SRC_FILES +=  project.cpp
LOCAL_LDLIBS += -llog

include $(BUILD_SHARED_LIBRARY)