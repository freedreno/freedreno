LOCAL_PATH := $(call my-dir)

#
# Build libwrap:
#

include $(CLEAR_VARS)
LOCAL_MODULE	:= libwrap
LOCAL_SRC_FILES	:= wrap/wrap-util.c wrap/wrap-syscall.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_LDLIBS := -llog -lc -ldl
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libwrapfake
LOCAL_SRC_FILES := wrap/wrap-util.c wrap/wrap-syscall-fake.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_LDLIBS := -llog -lc -ldl
include $(BUILD_SHARED_LIBRARY)


#
# 3D Test Apps:
#

include $(CLEAR_VARS)
LOCAL_MODULE    := test-advanced-blend
LOCAL_SRC_FILES := tests-3d/test-advanced-blend.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-bandwidth
LOCAL_SRC_FILES := tests-3d/test-bandwidth.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-blend-fbo
LOCAL_SRC_FILES := tests-3d/test-blend-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-caps
LOCAL_SRC_FILES := tests-3d/test-caps.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-cat
LOCAL_SRC_FILES := tests-3d/test-cat.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-clear
LOCAL_SRC_FILES := tests-3d/test-clear.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-clear-fbo
LOCAL_SRC_FILES := tests-3d/test-clear-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-compiler
LOCAL_SRC_FILES := tests-3d/test-compiler.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-compute
LOCAL_SRC_FILES := tests-3d/test-compute.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-cube
LOCAL_SRC_FILES := tests-3d/test-cube.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-cubemap
LOCAL_SRC_FILES := tests-3d/test-cubemap.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-cube-textured
LOCAL_SRC_FILES := tests-3d/test-cube-textured.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-draw
LOCAL_SRC_FILES := tests-3d/test-draw.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-draw-indirect
LOCAL_SRC_FILES := tests-3d/test-draw-indirect.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-enable-disable
LOCAL_SRC_FILES := tests-3d/test-enable-disable.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-es2gears
LOCAL_SRC_FILES := tests-3d/test-es2gears.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-float-int
LOCAL_SRC_FILES := tests-3d/test-float-int.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-frag-depth
LOCAL_SRC_FILES := tests-3d/test-frag-depth.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-image
LOCAL_SRC_FILES := tests-3d/test-image.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-instanced
LOCAL_SRC_FILES := tests-3d/test-instanced.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-int-varyings
LOCAL_SRC_FILES := tests-3d/test-int-varyings.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-layout-2d-array
LOCAL_SRC_FILES := tests-3d/test-layout-2d-array.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-limits
LOCAL_SRC_FILES := tests-3d/test-limits.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-lrz
LOCAL_SRC_FILES := tests-3d/test-lrz.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-mip-fbo
LOCAL_SRC_FILES := tests-3d/test-mip-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-mipmap
LOCAL_SRC_FILES := tests-3d/test-mipmap.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-mrt-fbo
LOCAL_SRC_FILES := tests-3d/test-mrt-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-msaa
LOCAL_SRC_FILES := tests-3d/test-msaa.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-noattach-fbo
LOCAL_SRC_FILES := tests-3d/test-noattach-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-perf
LOCAL_SRC_FILES := tests-3d/test-perf.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-piglit-bad
LOCAL_SRC_FILES := tests-3d/test-piglit-bad.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-piglit-good
LOCAL_SRC_FILES := tests-3d/test-piglit-good.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-quad-attributeless
LOCAL_SRC_FILES := tests-3d/test-quad-attributeless.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-quad-flat2
LOCAL_SRC_FILES := tests-3d/test-quad-flat2.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-quad-flat
LOCAL_SRC_FILES := tests-3d/test-quad-flat.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-quad-flat-fbo
LOCAL_SRC_FILES := tests-3d/test-quad-flat-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-quad-textured2
LOCAL_SRC_FILES := tests-3d/test-quad-textured2.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-quad-textured-3d
LOCAL_SRC_FILES := tests-3d/test-quad-textured-3d.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-quad-textured
LOCAL_SRC_FILES := tests-3d/test-quad-textured.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-query
LOCAL_SRC_FILES := tests-3d/test-query.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-restore-resolve
LOCAL_SRC_FILES := tests-3d/test-restore-resolve.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-sampshad
LOCAL_SRC_FILES := tests-3d/test-sampshad.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := shader-runner
LOCAL_SRC_FILES := tests-3d/test-shader-runner.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-shader-size
LOCAL_SRC_FILES := tests-3d/test-shader-size.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-srgb-fbo
LOCAL_SRC_FILES := tests-3d/test-srgb-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-ssbo
LOCAL_SRC_FILES := tests-3d/test-ssbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-stencil
LOCAL_SRC_FILES := tests-3d/test-stencil.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-strip-smoothed
LOCAL_SRC_FILES := tests-3d/test-strip-smoothed.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-tex
LOCAL_SRC_FILES := tests-3d/test-tex.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-tex-fbo
LOCAL_SRC_FILES := tests-3d/test-tex-fbo.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-tex-layout
LOCAL_SRC_FILES := tests-3d/test-tex-layout.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-tex-msaa
LOCAL_SRC_FILES := tests-3d/test-tex-msaa.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-texturator
LOCAL_SRC_FILES := tests-3d/test-texturator.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-tf
LOCAL_SRC_FILES := tests-3d/test-tf.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-triangle-quad
LOCAL_SRC_FILES := tests-3d/test-triangle-quad.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-triangle-smoothed
LOCAL_SRC_FILES := tests-3d/test-triangle-smoothed.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-ubwc
LOCAL_SRC_FILES := tests-3d/test-ubwc.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-varyings
LOCAL_SRC_FILES := tests-3d/test-varyings.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv2
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-vertex
LOCAL_SRC_FILES := tests-3d/test-vertex.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC -std=c99
LOCAL_LDLIBS := -llog -lc -ldl -lEGL -lGLESv3
include $(BUILD_EXECUTABLE)

#
# 2D Test Apps:
#

include $(CLEAR_VARS)
LOCAL_MODULE    := test-composite2
LOCAL_SRC_FILES := tests-2d/test-composite2.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-composite
LOCAL_SRC_FILES := tests-2d/test-composite.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-copy
LOCAL_SRC_FILES := tests-2d/test-copy.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-fb
LOCAL_SRC_FILES := tests-2d/test-fb.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-fill2
LOCAL_SRC_FILES := tests-2d/test-fill2.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-fill
LOCAL_SRC_FILES := tests-2d/test-fill.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-multi
LOCAL_SRC_FILES := tests-2d/test-multi.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-replay
LOCAL_SRC_FILES := tests-2d/test-replay.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE    := test-ubwc2d
LOCAL_SRC_FILES := tests-2d/test-ubwc2d.c tests-2d/c2d2-shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -ldl -llog -lm
include $(BUILD_EXECUTABLE)

#
# CL Test Apps for db820c.  Requires a local copy of the vendor image,
# so disabled by default.
#
ifeq (0,1)

include $(CLEAR_VARS)
# TODO this needs to be diff for 32 vs 64 bit
P = ~/src/db820c/z4/system-z4-full
LOCAL_MODULE    := test-imagecl
LOCAL_SRC_FILES := tests-cl/test-imagecl.c tests-cl/shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -lc++ -ldl -llog -lm
LOCAL_LDLIBS := $(P)/vendor/lib64/libOpenCL.so -rpath $(P)/vendor/lib64 -rpath $(P)/lib64
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
# TODO this needs to be diff for 32 vs 64 bit
P = ~/src/db820c/z4/system-z4-full
LOCAL_MODULE    := test-kernel
LOCAL_SRC_FILES := tests-cl/test-kernel.c tests-cl/shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -lc++ -ldl -llog -lm
LOCAL_LDLIBS := $(P)/vendor/lib64/libOpenCL.so -rpath $(P)/vendor/lib64 -rpath $(P)/lib64
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
# TODO this needs to be diff for 32 vs 64 bit
P = ~/src/db820c/z4/system-z4-full
LOCAL_MODULE    := test-simple
LOCAL_SRC_FILES := tests-cl/test-simple.c tests-cl/shim.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/includes $(LOCAL_PATH)/util
LOCAL_CFLAGS := -DBIONIC
LOCAL_LDLIBS := -lc -lc++ -ldl -llog -lm
LOCAL_LDLIBS := $(P)/vendor/lib64/libOpenCL.so -rpath $(P)/vendor/lib64 -rpath $(P)/lib64
include $(BUILD_EXECUTABLE)

endif
