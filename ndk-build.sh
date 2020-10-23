export NDK_PROJECT_PATH=`pwd`
ANDROID_NDK_ROOT=${ANDROID_NDK_ROOT:-$HOME/android-ndk-r13b}
$ANDROID_NDK_ROOT/build/ndk-build NDK_APPLICATION_MK=./Application.mk "$@"
