APP_STL := c++_static
NDK_TOOLCHAIN_VERSION := clang

PROJDIR		:= $(PWD)

DEPLOY_BUILD_ANDROID_ARM64 = $(PROJDIR)/build/android-arm64/lib/libavs.so

ifneq ($(wildcard $(DEPLOY_BUILD_ANDROID_ARM64)), )
$(info application.mk 2 android-arm64 no exist DEPLOY_BUILD_ANDROID_ARM64:$(DEPLOY_BUILD_ANDROID_ARM64)) 
# APP_ABI := armeabi-v7a
APP_ABI := armeabi-v7a arm64-v8a
else
$(info application.mk 1 android-arm64 exist DEPLOY_BUILD_ANDROID_ARM64:$(DEPLOY_BUILD_ANDROID_ARM64)) 
APP_ABI := armeabi-v7a arm64-v8a
endif

APP_PLATFORM := android-16
APP_PIE := false
