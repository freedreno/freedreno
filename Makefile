# Note: this VPATH bit is just a hack to avoid mucking with the build
# system after splitting the directory tree into various subdirs..
# really need to clean this up but I've got better things to work on
# right now:
VPATH = tests-2d:tests-3d:tests-cl:util:wrap

TESTS_2D = \
	test-fill \
	test-fill2 \
	test-replay \
	test-copy \
	test-fb \
	test-composite \
	test-composite2 \
	test-multi

TESTS_3D = \
	test-advanced-blend \
	test-bandwidth \
	test-blend-fbo \
	test-caps \
	test-cat \
	test-clear \
	test-clear-fbo \
	test-compiler \
	test-compute \
	test-cube \
	test-cube-textured \
	test-cubemap \
	test-draw \
	test-draw-indirect \
	test-enable-disable \
	test-es2gears \
	test-float-int \
	test-frag-depth \
	test-image \
	test-instanced \
	test-int-varyings \
	test-layout-2d-array \
	test-limits \
	test-lrz \
	test-mip-fbo \
	test-mipmap \
	test-mrt-fbo \
	test-msaa \
	test-noattach-fbo \
	test-perf \
	test-piglit-bad \
	test-piglit-good \
	test-quad-attributeless \
	test-quad-flat \
	test-quad-flat2 \
	test-quad-flat-fbo \
	test-quad-textured \
	test-quad-textured2 \
	test-quad-textured-3d \
	test-query \
	test-restore-resolve \
	test-sampshad \
	test-shader-runner \
	test-shader-size \
	test-srgb-fbo \
	test-ssbo \
	test-stencil \
	test-strip-smoothed \
	test-tex \
	test-tex-fbo \
	test-tex-layout \
	test-tf \
	test-triangle-quad \
	test-triangle-smoothed \
	test-ubo \
	test-ubwc \
	test-varyings \
	test-vertex

TESTS_CL = \
	test-simple \
	test-image

TESTS = $(TESTS_2D) $(TESTS_3D) $(TESTS_CL)
UTILS = bmp.o

CFLAGS = -Iincludes -Iutil

# Build Mode:
#  bionic -  build for gnu/linux style filesystem, linking
#            against android libc and libs in /system/lib
#  glibc  -  build for gnu/linux glibc, linking against
#            normal gnu dynamic loader
BUILD ?= bionic

# Window System (only used for BUILD == glibc):
# x11     - build for X11
# wayland - build for wayland
WINDOW_SYSTEM ?= x11

ifeq ($(strip $(BUILD)),bionic)
# Note: setup symlinks in /system/lib to the vendor specific .so's in
# /system/lib/egl because android's dynamic linker can't seem to cope
# with multiple -rpath's..
# Possibly we don't need to link directly against gpu specific libs
# but I was getting eglCreateContext() failing otherwise.
LFLAGS_3D = -lEGL_adreno200 -lGLESv2_adreno200
LFLAGS_2D = -lC2D2 -lOpenVG
LFLAGS_CL = -lOpenCL
LDFLAGS_MISC = -lgsl -llog -lcutils -lstdc++ -lstlport -lm
CFLAGS += -DBIONIC
CC = gcc -L /system/lib -mfloat-abi=soft
LD = ld --entry=_start -nostdlib --dynamic-linker /system/bin/linker -rpath /system/lib -L /system/lib
# only build c2d2 bits for android, otherwise we don't have the right
# headers/libs:
WRAP_C2D2 = wrap-c2d2.o
else ifeq ($(strip $(BUILD)),glibc)

ifeq ($(strip $(WINDOW_SYSTEM)),x11)
CFLAGS += -DSUPPORT_X11
LDFLAGS_MISC = -lX11 -lm
else ifeq ($(strip $(WINDOW_SYSTEM)),wayland)
CFLAGS += -DSUPPORT_WAYLAND
LDFLAGS_MISC = -lwayland-client -lm
endif

LFLAGS_3D = -lEGL -lGLESv2
LFLAGS_2D =
#LFLAGS_CL = -lOpenCL
CC = gcc -L /usr/lib
LD = gcc -L /usr/lib
WRAP_C2D2 =
else
error "Invalid build type"
endif

LFLAGS = $(LFLAGS_2D) $(LFLAGS_3D) $(LFLAGS_CL) $(LDFLAGS_MISC) -ldl -lc

all: tests-3d tests-2d tests-cl

utils: libwrap.so $(UTILS) redump zdump

tests-2d: $(TESTS_2D)

tests-3d: $(TESTS_3D)

tests-cl: $(TESTS_CL)

clean:
	rm -f *.bmp *.dat *.so *.o *.rd *.html *.log redump $(TESTS)

wrap%.o: wrap%.c
	$(CC) -fPIC -g -c -ldl -llog -c -Iincludes -Iutil $< -o $@

%.o: %.c
	$(CC) -fPIC -g -c $(CFLAGS) $(LFLAGS) $< -o $@

libwrap.so: wrap-util.o wrap-syscall.o $(WRAP_C2D2)
	$(LD) -shared -ldl -lc -llog $^ -o $@

libwrapfake.so: wrap-util.o wrap-syscall-fake.o
	$(LD) -shared -ldl -lc -llog $^ -o $@

test-%: test-%.o $(UTILS)
	$(LD) $^ $(LFLAGS) -o $@

# build redump normally.. it doesn't need to link against android libs
redump: redump.c
	gcc -g $^ -o $@

zdump: zdump.c
	gcc -g $(CFLAGS) -Wall -Wno-packed-bitfield-compat -I. $^ -o $@

