#
# Android.mk
#

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

PROJDIR		:= $(PWD)
BUILDDEBUG	:= 1

LOCAL_MODULE    := avs
LOCAL_CFLAGS    := -g -DHAVE_INTTYPES_H=1 -DPOSIX -DHAVE_WEBRTC \
		   -DWEBRTC_ANDROID -DDEBUG=$(BUILDDEBUG) \
		   -pthread
LOCAL_CXXFLAGS   := -g -DHAVE_INTTYPES_H=1 -DPOSIX -DHAVE_WEBRTC \
		   -x c++ -std=c++11 -stdlib=libc++ \
		   -DWEBRTC_ANDROID -DDEBUG=$(BUILDDEBUG) \
		   -pthread

DEPLOY_BUILD_ANDROID_ARM64 = $(PROJDIR)/build/android-arm64/lib/libavs.so
BUILD_ANDROID_PATH = 

ifneq ($(wildcard $(DEPLOY_BUILD_ANDROID_ARM64)),)
$(info android.mk 0 qita DEPLOY_BUILD_ANDROID_ARM64:$(DEPLOY_BUILD_ANDROID_ARM64) ,TARGET_ARCH_ABI:$(TARGET_ARCH_ABI)) 
# BUILD_ANDROID_PATH = android-armv7
BUILD_ANDROID_PATH = android-arm64
else
$(info android.mk 0 android-arm64 DEPLOY_BUILD_ANDROID_ARM64:$(DEPLOY_BUILD_ANDROID_ARM64),TARGET_ARCH_ABI:$(TARGET_ARCH_ABI)) 
BUILD_ANDROID_PATH = android-arm64
endif

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
$(info android.mk arm64-v8a 1 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI) ,BUILD_ANDROID_PATH:$(BUILD_ANDROID_PATH)) 
LOCAL_C_INCLUDES := ../build/$(BUILD_ANDROID_PATH)/include \
		    ../build/$(BUILD_ANDROID_PATH)/include/re \
		    ../include \
		    ../mediaengine
else
$(info android.mk qita 1 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI) ,BUILD_ANDROID_PATH:$(BUILD_ANDROID_PATH)) 
LOCAL_C_INCLUDES := ../build/android-armv7/include \
		    ../build/android-armv7/include/re \
		    ../include \
		    ../mediaengine
endif

LOCAL_SRC_FILES := \
		audio_effect.cc \
		flow_manager.cc \
		media_manager.cc \
		video_renderer.cc

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
$(info android.mk arm64-v8a 2 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI) ,BUILD_ANDROID_PATH:$(BUILD_ANDROID_PATH)) 
LOCAL_LDLIBS    := \
		-L../build/$(BUILD_ANDROID_PATH)/lib \
		../build/$(BUILD_ANDROID_PATH)/lib/libavscore.a \
		-lvpx \
		-lusrsctp \
		-lre \
		-lrew \
		-lsodium \
		-lssl \
		-lcrypto \
		-lcpufeatures \
		-llog -lz -lGLESv2 \
		-latomic
else
$(info android.mk qita 2 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI) ,BUILD_ANDROID_PATH:$(BUILD_ANDROID_PATH)) 
LOCAL_LDLIBS    := \
		-L../build/android-armv7/lib \
		../build/android-armv7/lib/libavscore.a \
		-lvpx \
		-lusrsctp \
		-lre \
		-lrew \
		-lsodium \
		-lssl \
		-lcrypto \
		-lcpufeatures \
		-llog -lz -lGLESv2 \
		-latomic
endif

LOCAL_C_INCLUDES += \
		../mediaengine

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
$(info android.mk arm64-v8a 3 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI) ,BUILD_ANDROID_PATH:$(BUILD_ANDROID_PATH)) 
LOCAL_LDLIBS    += \
		-L../../build/$(BUILD_ANDROID_PATH)/lib \
		-lmediaengine \
		-lvpx \
		-lcpufeatures \
		-lopus \
		-lOpenSLES \
		-llog
else
$(info android.mk qita 3 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI) ,BUILD_ANDROID_PATH:$(BUILD_ANDROID_PATH)) 
LOCAL_LDLIBS    += \
		-L../../build/android-armv7/lib \
		-lmediaengine \
		-lvpx \
		-lcpufeatures \
		-lopus \
		-lOpenSLES \
		-llog
endif

ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
$(info android.mk arm64-v8a 4 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI)) 
LOCAL_LDLIBS	+= \
		$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/arm64-v8a/libc++_static.a \
		$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/arm64-v8a/libc++abi.a \
		$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/arm64-v8a/libandroid_support.a
else
$(info android.mk qita 4 TARGET_ARCH_ABI:$(TARGET_ARCH_ABI)) 
LOCAL_LDLIBS	+= \
		$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/libc++_static.a \
		$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/libc++abi.a \
		$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/libandroid_support.a \
		$(ANDROID_NDK_ROOT)/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/libunwind.a
endif

include $(BUILD_SHARED_LIBRARY)
