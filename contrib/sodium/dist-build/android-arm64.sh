#!/bin/sh
export TARGET_ARCH=arm64
export CFLAGS="-Os -march=${TARGET_ARCH}"
CC="aarch64-linux-android-gcc" NDK_PLATFORM=android-21 ARCH=arm64 HOST_COMPILER=aarch64-linux-android "$(dirname "$0")/android-build.sh"
