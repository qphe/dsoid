APP_PLATFORM := android-9
APP_ABI := armeabi-v7a
APP_CPPFLAGS := -O3 -ffast-math -fomit-frame-pointer -fno-math-errno -funsafe-math-optimizations -ffinite-math-only -fbranch-target-load-optimize -Wno-psabi -flto
APP_STL := gnustl_static
NDK_TOOLCHAIN_VERSION=4.7
#APP_OPTIM := debug
#APP_CPPFLAGS := -O0