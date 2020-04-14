/*
 * Copyright (c) 2011 Intel Corporation
 * Copyright (c) 2019 Rob Clark <robdclark@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdbool.h>
#include <getopt.h>
#include <GLES3/gl31.h>

#include "test-util-3d.h"

/* A tool for debugging texture layout.  Somewhat inspired by piglit's
 * texelFetch, but with some differences:
 *
 *  - Uses GLES3+ and kms/gbm to reduce dependencies.
 *  - Only samples from FS stage, since this simplifies things and
 *    testing from VS/GS is not important for texture layout
 *  - Encodes the slice and mipmap level as the texture contents;
 *    since this is mostly for making sure we have the slice and
 *    level offsets to put he pixel data where the hw expects to
 *    read it from, the x/y gradient is less useful, but knowing
 *    the slice/level that was actually read helps give better
 *    error messages.  (Once this is working correctly, you can
 *    go off and run texelFetch to make sure you didn't get the
 *    tiling format wrong, etc.)
 *  - Supports all different formats, since the rules for calculate
 *    level/slice offset can differ, or be parameterized differently,
 *    for different formats.
 *  - multiple levels of zoom.. samples same tex coord for NxN screen
 *    coordinates, to more easily see smaller mipmap levels on hidpi
 *    screens.
 *
 * Like texelFetch, it also supports:
 *  - 2D, 3D, 2DArray
 *  - Mipmapping
 *
 * Description of layout on screen from texelFetch:
 *
 * Draws a series of "rectangles" which display each miplevel and array slice,
 * at full size.  They are layed out as follows:
 *
 * miplevel 3 +          +          +          +          +
 *
 * miplevel 2 +-+        +-+        +-+        +-+        +-+
 *            +-+        +-+        +-+        +-+        +-+
 *
 * miplevel 1 +---+      +---+      +---+      +---+      +---+
 *            |   |      |   |      |   |      |   |      |   |
 *            +---+      +---+      +---+      +---+      +---+
 *
 *            +------+   +------+   +------+   +------+   +------+
 * miplevel 0 |      |   |      |   |      |   |      |   |      |
 *            |      |   |      |   |      |   |      |   |      |
 *            +------+   +------+   +------+   +------+   +------+
 *            slice #0   slice #1   slice #2   slice #3   slice #4
 *
 */

static int max_error_frames = 5;
static int error_frames;
static int zoom = 1;
static bool full;
static bool stop;
static bool png;
static GLenum target;
static struct size {
	unsigned x, y, z;
} size, minsz, maxsz;
int miplevels;
static GLint screen_width, screen_height;

static bool is_array(GLenum target)
{
	return target == GL_TEXTURE_2D_ARRAY;
}

static int get_ncomp(GLenum ufmt)
{
	switch (ufmt) {
	case GL_RED:
	case GL_RED_INTEGER:
	case GL_DEPTH_COMPONENT:
		return 1;
	case GL_RG:
	case GL_RG_INTEGER:
	case GL_DEPTH_STENCIL:
		return 2;
	case GL_RGB:
	case GL_RGB_INTEGER:
		return 3;
	case GL_RGBA:
	case GL_RGBA_INTEGER:
		return 4;
	default:
		assert(!"bad format");
		return 0;
	}
}

/*
 * Formats Table
 * -------------
 *
 * The worst case (smallest number of bits) to encode slice and level # are
 * the R8 formats, but 4 bits for each is sufficient precision.  To simplify
 * things we standardize[*] how this is encoded:
 *
 *    +----------+--------------+--------------------+
 *    | type     | range used   | GLenum type        |
 *    +----------+--------------+--------------------+
 *    |  SNORM   |  -1.0..1.0   |  GL_BYTE           |
 *    |  UNORM   |   0.0..1.0   |  GL_UNSIGNED_BYTE  |
 *    |  FLOAT   |   0.0..1.0   |  GL_FLOAT          |
 *    |  SINT8   |  -128..127   |  GL_BYTE           |
 *    |  UINT8   |  -128..127   |  GL_UNSIGNED_BYTE  |
 *    |  SINT16  |  -128..127   |  GL_SHORT          |
 *    |  UINT16  |  -128..127   |  GL_UNSIGNED_SHORT |
 *    |  SINT32  |  -128..127   |  GL_INT            |
 *    |  UINT32  |  -128..127   |  GL_UNSIGNED_INT   |
 *    +----------+--------------+--------------------+
 *
 * [*] The "oddball" formats like RGB565 will need an additional level of
 *     "packing" to encode the 8bit slice+level across multiple channels.
 *     Other formats simply replicate the same value across all channels.
 *
 * The shader unpacks the slice/level value and returns them as gl_FragColor
 * red and green channels.
 */
enum type {
	SNORM,
	UNORM,
	FLOAT,
	SINT8,
	UINT8,
	SINT16,
	UINT16,
	SINT32,
	UINT32,
	// TODO add oddballs
};

static int enc_ls(int level, int slice)
{
	return ((level << 3) & 0x78) | (slice & 0x7);
}

static void * encode_BYTE(void *buf, int ncomp, int w, int h, int level, int slice)
{
	int8_t *ptr = buf;
	int8_t val = enc_ls(level, slice) - 127;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < (w*ncomp); j++)
			*(ptr++) = val;
		val += (i & 1) ? -128 : 128;
	}

	return ptr;
}

static void * encode_UNSIGNED_BYTE(void *buf, int ncomp, int w, int h, int level, int slice)
{
	uint8_t *ptr = buf;
	uint8_t val = enc_ls(level, slice);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < (w*ncomp); j++)
			*(ptr++) = val;
		val += (i & 1) ? -128 : 128;
	}

	return ptr;
}

static void * encode_FLOAT(void *buf, int ncomp, int w, int h, int level, int slice)
{
	float *ptr = buf;
	int encoded_value = enc_ls(level, slice);
	float original_value = ((float)encoded_value) / 255.0;
	float complement_value = ((float)(encoded_value + 128)) / 255.0;
	float val;

	for (int i = 0; i < h; i++) {
		val = (i & 1) ? complement_value : original_value;
		for (int j = 0; j < (w*ncomp); j++)
			*(ptr++) = val;
	}

	return ptr;
}

static void * encode_SHORT(void *buf, int ncomp, int w, int h, int level, int slice)
{
	int16_t *ptr = buf;
	int16_t val = enc_ls(level, slice) - 127;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < (w*ncomp); j++)
			*(ptr++) = val;
		val += (i & 1) ? -128 : 128;
	}

	return ptr;
}

static void * encode_UNSIGNED_SHORT(void *buf, int ncomp, int w, int h, int level, int slice)
{
	uint16_t *ptr = buf;
	uint16_t val = enc_ls(level, slice);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < (w*ncomp); j++)
			*(ptr++) = val;
		val += (i & 1) ? -128 : 128;
	}

	return ptr;
}

static void * encode_INT(void *buf, int ncomp, int w, int h, int level, int slice)
{
	int32_t *ptr = buf;
	int32_t val = enc_ls(level, slice) - 127;

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < (w*ncomp); j++)
			*(ptr++) = val;
		val += (i & 1) ? -128 : 128;
	}
	return ptr;
}

static void * encode_UNSIGNED_INT(void *buf, int ncomp, int w, int h, int level, int slice)
{
	uint32_t *ptr = buf;
	uint32_t val = enc_ls(level, slice);

	for (int i = 0; i < h; i++) {
		for (int j = 0; j < (w*ncomp); j++)
			*(ptr++) = val;
		val += (i & 1) ? -128 : 128;
	}

	return ptr;
}

static struct type_info {
	const char *unpack;
	const char *convert;
	GLenum type;
	void * (*encode)(void *buf, int ncomp, int w, int h, int level, int slice);
} type_info[] = {
#define _TYPE(_name, _unpack, _convert, _type) \
	[_name] = { _unpack, _convert, GL_ ## _type, encode_ ## _type }
/* for the simple types that just encode value in .r channel: */
#define STYPE(_name, _convert, _type) _TYPE(_name, "color.r", _convert, _type)

	STYPE(SNORM,  "(val + 1.0) * 127.0", BYTE),
	STYPE(UNORM,  "val * 255.0",         UNSIGNED_BYTE),
	STYPE(FLOAT,  "val * 255.0",         FLOAT),
	STYPE(SINT8,  "val + 127",           BYTE),
	STYPE(UINT8,  "val",                 UNSIGNED_BYTE),
	STYPE(SINT16, "val + 127",           SHORT),
	STYPE(UINT16, "val",                 UNSIGNED_SHORT),
	STYPE(SINT32, "val + 127",           INT),
	STYPE(UINT32, "val",                 UNSIGNED_INT),
};

static const struct fmt {
	const char *name;
	GLenum ifmt;    /* sized internal format */
	GLenum ufmt;    /* unsized format */
	enum type type;
} fmts[] = {
#define FMT(name, ufmt, t) { #name, GL_##name, ufmt, t }
	FMT(R8,            GL_RED,           UNORM),
	FMT(R8UI,          GL_RED_INTEGER,   UINT8 ),
	FMT(R8I,           GL_RED_INTEGER,   SINT8 ),
	FMT(R16UI,         GL_RED_INTEGER,   UINT16),
	FMT(R16I,          GL_RED_INTEGER,   SINT16),
	FMT(R32UI,         GL_RED_INTEGER,   UINT32),
	FMT(R32I,          GL_RED_INTEGER,   SINT32),
	FMT(RG8,           GL_RG,            UNORM ),
	FMT(RG8UI,         GL_RG_INTEGER,    UINT8 ),
	FMT(RG8I,          GL_RG_INTEGER,    SINT8 ),
	FMT(RG16UI,        GL_RG_INTEGER,    UINT16),
	FMT(RG16I,         GL_RG_INTEGER,    SINT16),
	FMT(RG32UI,        GL_RG_INTEGER,    UINT32),
	FMT(RG32I,         GL_RG_INTEGER,    SINT32),
	FMT(RGB8,          GL_RGB,           UNORM ),
//	FMT(RGB565,        GL_RGB,           UNORM565),
	FMT(RGBA8,         GL_RGBA,          UNORM ),
//	FMT(RGB5_A1,       GL_RGBA,          UINT5551),
//	FMT(RGBA4,         GL_RGBA,          UINT4444),
//	FMT(RGB10_A2,      GL_RGBA,          UINT10A2),
	FMT(RGBA8UI,       GL_RGBA_INTEGER,  UINT8 ),
	FMT(RGBA8I,        GL_RGBA_INTEGER,  SINT8 ),
	FMT(RGBA16UI,      GL_RGBA_INTEGER,  UINT16),
	FMT(RGBA16I,       GL_RGBA_INTEGER,  SINT16),
	FMT(RGBA32I,       GL_RGBA_INTEGER,  SINT32),
	FMT(RGBA32UI,      GL_RGBA_INTEGER,  UINT32),
	/* Not required to be color renderable: */
	FMT(R8_SNORM,       GL_RED,          SNORM ),
	FMT(R16F,           GL_RED,          FLOAT ),
	FMT(R32F,           GL_RED,          FLOAT ),
	FMT(RG8_SNORM,      GL_RG,           SNORM ),
	FMT(RG16F,          GL_RG,           FLOAT ),
	FMT(RG32F,          GL_RG,           FLOAT ),
	FMT(SRGB8,          GL_RGB,          UNORM ),
	FMT(RGB8_SNORM,     GL_RGB,          SNORM ),
	FMT(R11F_G11F_B10F, GL_RGB,          FLOAT ),
	FMT(RGB9_E5,        GL_RGB,          FLOAT ),
	FMT(RGB16F,         GL_RGB,          FLOAT ),
	FMT(RGB32F,         GL_RGB,          FLOAT ),
	FMT(RGB8UI,         GL_RGB_INTEGER,  UINT8 ),
	FMT(RGB8I,          GL_RGB_INTEGER,  SINT8 ),
	FMT(RGB16UI,        GL_RGB_INTEGER,  UINT16),
	FMT(RGB16I,         GL_RGB_INTEGER,  SINT16),
	FMT(RGB32UI,        GL_RGB_INTEGER,  UINT32),
	FMT(RGB32I,         GL_RGB_INTEGER,  SINT32),
	FMT(RGBA8_SNORM,    GL_RGBA,         UNORM ),
	FMT(RGBA16F,        GL_RGBA,         FLOAT ),
	FMT(RGBA32F,        GL_RGBA,         FLOAT ),
	FMT(DEPTH_COMPONENT16,  GL_DEPTH_COMPONENT, UINT16),
	FMT(DEPTH_COMPONENT24,  GL_DEPTH_COMPONENT, UINT32),
	FMT(DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, FLOAT ),
//	FMT(DEPTH24_STENCIL8,   GL_DEPTH_STENCIL,   UINTZ24S8),
//	FMT(DEPTH32F_STENCIL8,  GL_DEPTH_STENCIL,   FLOATZ32UINTX24S8),
};

static const struct fmt *fmt;

/*
 * Shaders:
 */

static const char *get_prefix(void)
{
	switch(fmt->type) {
	case SINT8:
	case SINT16:
	case SINT32:
		return "i";
	case UINT8:
	case UINT16:
	case UINT32:
		return "u";
	default:
		return "";
	}
}

static const char *get_sampler(void)
{
	static char buf[32];
	const char *stype;

	if (target == GL_TEXTURE_2D) {
		stype = "2D";
	} else if (target == GL_TEXTURE_2D_ARRAY) {
		stype = "2DArray";
	} else if (target == GL_TEXTURE_3D) {
		stype = "3D";
	} else {
		assert(!"bad mode!");
	}

	sprintf(buf, "%ssampler%s", get_prefix(), stype);
	return buf;
}

#define IN_POSITION 0
#define IN_TEXCOORD 1
const char *vertex_shader_source =
	"#version 300 es                           \n"
	"in vec4 in_position;                      \n"
	"in vec4 in_texcoord;                      \n"
	"out vec4 v_texcoord;                      \n"
	"void main()                               \n"
	"{                                         \n"
	"	v_texcoord = in_texcoord;          \n"
	"	gl_Position = in_position;         \n"
	"}                                         \n";

const char *fragment_shader_fmt =
	"#version 300 es                                                    \n"
	"#define ivec1 int                                                  \n"
	"#define uvec1 uint                                                 \n"
	"#define vec1 float                                                 \n"
	"precision highp float;                                             \n"
	"precision highp int;                                               \n"
	"in vec4 v_texcoord;                                                \n"
	"uniform highp %1$s tex;                                            \n"
	"out vec4 fragColor;                                                \n"
	"void main()                                                        \n"
	"{                                                                  \n"
	"	int lod = int(v_texcoord.w);                                \n"
	"	%2$svec4 color = texelFetch(tex, ivec%3$d(v_texcoord), lod);\n"
	"	%2$svec1 val = %4$s;                                        \n"
	"	int converted = int(%5$s);                                  \n"
	"	fragColor.rgb = vec3(                                       \n"
	"		float((converted >> 0) & 0x7) /  8.0,               \n"
	"		float((converted >> 3) & 0xf) / 16.0,               \n"
	"		float((converted >> 7) & 0x1) *  0.25                \n"
	"	);                                                          \n"
	"	fragColor.a = 1.0;                                          \n"
	"}                                                                  \n";

static const char *get_fs(void)
{
	static char buf[4096];
	int ncoord = (target == GL_TEXTURE_2D) ? 2 : 3;
	unsigned n;

	n = sprintf(buf, fragment_shader_fmt, get_sampler(),
		get_prefix(), ncoord,
		type_info[fmt->type].unpack,
		type_info[fmt->type].convert);
	assert(n < sizeof(buf) - 1);

	return buf;
}

static void upload_texture(void)
{
	int ncomp = get_ncomp(fmt->ufmt);
	char *texbuf = malloc(size.x * size.y * size.z * ncomp * 16);
	struct type_info *ti = &type_info[fmt->type];

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	for (int m = 0; m < miplevels; m++) {
		int w = u_minify(size.x, m);
		int h = u_minify(size.y, m);
		/* size in 3rd dim minifies for 3D but not 2D_ARRAY: */
		int slices = is_array(target) ? size.z : u_minify(size.z, m);
		void *ptr = texbuf;

		for (int s = 0; s < slices; s++) {
			ptr = ti->encode(ptr, ncomp, w, h, m, s);
		}

		switch (target) {
		case GL_TEXTURE_2D:
			glTexImage2D(target, m, fmt->ifmt, w, h, 0,
					fmt->ufmt, ti->type, texbuf);
			break;
		case GL_TEXTURE_3D:
		case GL_TEXTURE_2D_ARRAY:
			glTexImage3D(target, m, fmt->ifmt, w, h, slices, 0,
					fmt->ufmt, ti->type, texbuf);
			break;
		default:
			assert(!"bad target");
		}
	}
	free(texbuf);
}

static int tex_handle;
static unsigned tex;

static void setup_gl(void)
{
	int prog;

	prog = get_program(vertex_shader_source, get_fs());
	assert(prog >= 0);

	glBindAttribLocation(prog, IN_POSITION, "in_position");
	glBindAttribLocation(prog, IN_TEXCOORD, "in_texcoord");

	link_program(prog);

	glUseProgram(prog);

	glViewport(0, 0, screen_width, screen_height);

	glGenTextures(1, &tex);

	tex_handle = glGetUniformLocation(prog, "tex");
}

static void update_texture(void)
{
	/* calculate # of miplevels: */
	int max_dim;
	if (target == GL_TEXTURE_3D)
		max_dim = MAX2(size.x, MAX2(size.y, size.z));
	else
		max_dim = MAX2(size.x, size.y);

	miplevels = (int) log2f(max_dim) + 1;

	glDeleteTextures(1, &tex);
	glGenTextures(1, &tex);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	upload_texture();

	glUniform1i(tex_handle, 0); /* '0' refers to texture unit 0. */
}

static void draw_quad(float x, float y, float w, float h, float tw, float th, int m, int s)
{
	float in_position[4][4];
	float in_texcoord[4][4];

	/* convert from 0..1 to -1..1: */
	x = (x * 2.0) - 1.0;
	y = (y * 2.0) - 1.0;
	w *= 2.0;
	h *= 2.0;

	in_position[0][0] = x;
	in_position[0][1] = y;
	in_position[0][2] = 0.0;
	in_position[0][3] = 1.0;
	in_position[1][0] = x + w;
	in_position[1][1] = y;
	in_position[1][2] = 0.0;
	in_position[1][3] = 1.0;
	in_position[2][0] = x;
	in_position[2][1] = y + h;
	in_position[2][2] = 0.0;
	in_position[2][3] = 1.0;
	in_position[3][0] = x + w;
	in_position[3][1] = y + h;
	in_position[3][2] = 0.0;
	in_position[3][3] = 1.0;

	in_texcoord[0][0] = 0.0;
	in_texcoord[0][1] = 0.0;
	in_texcoord[0][2] = s;
	in_texcoord[0][3] = m;
	in_texcoord[1][0] = tw;
	in_texcoord[1][1] = 0.0;
	in_texcoord[1][2] = s;
	in_texcoord[1][3] = m;
	in_texcoord[2][0] = 0.0;
	in_texcoord[2][1] = th;
	in_texcoord[2][2] = s;
	in_texcoord[2][3] = m;
	in_texcoord[3][0] = tw;
	in_texcoord[3][1] = th;
	in_texcoord[3][2] = s;
	in_texcoord[3][3] = m;

	glVertexAttribPointer(IN_POSITION, 4, GL_FLOAT, GL_FALSE, 0, in_position);
	glEnableVertexAttribArray(IN_POSITION);

	glVertexAttribPointer(IN_TEXCOORD, 4, GL_FLOAT, GL_FALSE, 0, in_texcoord);
	glEnableVertexAttribArray(IN_TEXCOORD);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void extract_pix(uint8_t *rgba, int *slice, int *level, bool *complemented)
{
	*slice = (((float)rgba[0]) / 255.0) * 8.0;
	*level = (((float)rgba[1]) / 255.0) * 16.0;
	*complemented = (((float)rgba[2]) / 255.0) / 0.25;
}

static bool probe_pix(int x, int y, int w, int h, int s, int m)
{
	uint32_t rgba[w*h], *ptr;
	bool err = false;

	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

	ptr = rgba;
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			int slice, level;
			bool complemented;

			extract_pix((void *)(ptr++), &slice, &level, &complemented);
			if ((slice != s) || (level != m) || (complemented != ((i / zoom) & 1))) {
				printf("%ux%ux%u:%s: error at: S:L:C=%d:%d:%d, got %d:%d:%d at pix %d,%d (of %dx%d)\n",
					size.x, size.y, size.z, fmt->name,
					s, m, ((i / zoom) & 1), slice, level, complemented, j, i, w, h);
				err = true;
				if (!full)
					return err;
			}
		}
	}

	return err;
}

static bool check_quads(void)
{
	const int pad = 2;
	float y = pad;
	bool err = false;

	/* draw quads for each level/slice: */
	for (int m = 0; m < miplevels; m++) {
		float w = u_minify(size.x, m);
		float h = u_minify(size.y, m);
		/* size in 3rd dim minifies for 3D but not 2D_ARRAY: */
		int slices = is_array(target) ? size.z : u_minify(size.z, m);

		float x = pad;
		for (int s = 0; s < slices; s++) {
			int rx = x * zoom;
			int ry = y * zoom;

			if ((rx + w * zoom >= screen_width) || (ry + h * zoom >= screen_height))
				continue;

			err |= probe_pix(rx, ry, w*zoom, h*zoom, s, m);

			x += size.x + pad;
		}

		y += h + pad;
	}

	return err;
}

#ifdef HAVE_LIBPNG
static void write_png_file(char *filename, int width, int height, uint8_t *buffer)
{
	FILE *fp = fopen(filename, "wb");
	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop info = png_create_info_struct(png);

	png_init_io(png, fp);

	png_set_IHDR(
		png,
		info,
		width, height,
		8,
		PNG_COLOR_TYPE_RGBA,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE
	);
	png_write_info(png, info);

	png_bytepp rows = (png_bytepp)png_malloc(png, height * sizeof(png_bytep));
	for (int i = 0; i < height; i++)
    	rows[i] = (png_bytep)(buffer + (height - i - 1) * width * 4);

	png_write_image(png, rows);

	png_write_end(png, NULL);

	fclose(fp);
	png_destroy_write_struct(&png, &info);
  	free(rows);
}
#endif

static bool needs_check = true;

static EGLDisplay display;
static EGLConfig config;
static EGLint num_config;
static EGLContext context;

static void draw_and_check_quads()
{
	update_texture();

	/* clear the color buffer */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	float sw = screen_width;
	float sh = screen_height;

	sw /= zoom;
	sh /= zoom;

	const int pad = 2;
	float y = pad;

	/* draw quads for each level/slice: */
	for (int m = 0; m < miplevels; m++) {
		float w = u_minify(size.x, m);
		float h = u_minify(size.y, m);
		/* size in 3rd dim minifies for 3D but not 2D_ARRAY: */
		int slices = is_array(target) ? size.z : u_minify(size.z, m);

		float x = pad;
		for (int s = 0; s < slices; s++) {
			draw_quad(x/sw, y/sh, w/sw, h/sh, w, h, m, s);
			x += size.x + pad;
		}

		y += h + pad;
	}

	if (needs_check) {
		glFlush();
		bool err = check_quads();
		if (err)
			error_frames++;
		needs_check = false;

#ifdef HAVE_LIBPNG
		if (png) {
			uint8_t rgba[screen_width*screen_height*4];
			glReadPixels(0, 0, screen_width, screen_height, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
			static char buf[64];
			sprintf(buf, "kmscube-texturator-%dx%dx%d:%s.png",
					size.x, size.y, size.z, fmt->name);
			write_png_file(buf, screen_width, screen_height, rgba);
		}
#endif
	}

	/* if we've hit max # of error frames, stop growing: */
	if (error_frames >= max_error_frames)
		goto bail;

	if ((size.x < maxsz.x) || (size.y < maxsz.y) || (size.z < maxsz.z)) {
		/* Increase width first, then height, then depth: */
		size.x++;
		if (size.x > maxsz.x) {
			size.x = minsz.x;
			size.y++;
		}
		if (size.y > maxsz.y) {
			size.x = minsz.x;
			size.y = minsz.y;
			size.z++;
		}
		assert(size.z <= maxsz.z);
		needs_check = true;

		return;
	}

bail:
	if (stop) {
		printf("Exiting with %d errors\n", error_frames);
		exit((error_frames > 0) ? -1 : 0);
	}
}

static void print_summary(void)
{
	printf("testing %s %s at %ux%ux%u-%ux%ux%u with %dx zoom\n", fmt->name,
		get_sampler(), minsz.x, minsz.y, minsz.z,
		maxsz.x, maxsz.y, maxsz.z, zoom);
	printf("VS:\n%s\n", vertex_shader_source);
	printf("FS:\n%s\n", get_fs());
}

static const char *shortopts = "D:e:fsv:z";

static const struct option longopts[] = {
	{"errors", required_argument, 0, 'e'},
	{"full",   no_argument,       0, 'f'},
	{"stop",   no_argument,       0, 's'},
	{"zoom",   no_argument,       0, 'z'},
#ifdef HAVE_LIBPNG
	{"png",    no_argument,       0, 'p'},
#endif
	{0, 0, 0, 0}
};

static void usage(const char *name)
{
	printf("Usage: %1$s [-Dvz] <target> <format> <minsize> [<maxsize>]\n"
		"\n"
		"options:\n"
		"    -e, --errors=N       stop after N frames with errors (default 5)\n"
		"    -f, --full           check all pixels (do not stop after first faulty pixel)\n"
		"    -s, --stop           exit after testing all sizes\n"
		"    -z, --zoom           increase zoom (can be specified multiple times)\n"
#ifdef HAVE_LIBPNG
		"    -p, --png            capture the screen to a png image\n"
#endif
		"\n"
		"where:\n"
		"    <target>  is one of 2D/2DArray/3D\n"
		"    <format>  is a GL sized internal-format without GL_ prefix\n"
		"    <size>    is XxY (2D) or XxYxZ (2DArray/3D)\n"
		"\n"
		"example:\n"
		"    %1$s -z 3D RG16UI 37x65x4\n"
		, name);
	exit(-1);
}

static void parse_dims(const char *argv0, const char *sizestr, struct size *size)
{
	if (target == GL_TEXTURE_2D) {
		if (sscanf(sizestr, "%ux%ux", &size->x, &size->y) != 2) {
			printf("invalid size: %s\n", sizestr);
			usage(argv0);
		}
		size->z = 1;
	} else {
		if (sscanf(sizestr, "%ux%ux%ux", &size->x, &size->y, &size->z) != 3) {
			printf("invalid size: %s\n", sizestr);
			usage(argv0);
		}
	}
}

static EGLint const config_attribute_list[] = {
	EGL_RED_SIZE, 8,
	EGL_GREEN_SIZE, 8,
	EGL_BLUE_SIZE, 8,
	EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	EGL_DEPTH_SIZE, 8,
	EGL_NONE
};

static const EGLint context_attribute_list[] = {
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};

static void
parse_args(int argc, char *argv[])
{
	int opt;
	unsigned int len;

	while ((opt = getopt_long_only(argc, argv, shortopts, longopts, NULL)) != -1) {
		switch (opt) {
		case 'e':
			max_error_frames = atoi(optarg);
			break;
		case 'f':
			full = true;
			break;
		case 's':
			stop = true;
			break;
		case 'z':
			zoom++;
			break;
#ifdef HAVE_LIBPNG
		case 'p':
			png = true;
			break;
#endif
		default:
			usage(argv[0]);
		}
	}

	if ((optind + 2) >= argc) {
		usage(argv[0]);
	}

	/* parse remaining args: */
	const char *targetstr = argv[optind + 0];
	const char *fmtstr  = argv[optind + 1];
	const char *minstr  = argv[optind + 2];
	const char *maxstr  = ((optind + 3) < argc) ? argv[optind + 3] : NULL;

	if (!strcmp(targetstr, "2D")) {
		target = GL_TEXTURE_2D;
	} else if (!strcmp(targetstr, "2DArray")) {
		target = GL_TEXTURE_2D_ARRAY;
	} else if (!strcmp(targetstr, "3D")) {
		target = GL_TEXTURE_3D;
	} else {
		printf("invalid target: %s\n", targetstr);
		usage(argv[0]);
	}

	for (unsigned i = 0; i < ARRAY_SIZE(fmts); i++) {
		if (!strcmp(fmtstr, fmts[i].name)) {
			fmt = &fmts[i];
			break;
		}
	}

	if (!fmt) {
		printf("invalid format: %s\n", fmtstr);
		usage(argv[0]);
	}

	parse_dims(argv[0], minstr, &minsz);

	if (maxstr) {
		parse_dims(argv[0], maxstr, &maxsz);
	} else {
		maxsz = minsz;
	}

	size = minsz;
}

static void
run_texturator(void)
{
	RD_START("texturator", "Testing %dx%dx%d:%s\n", size.x, size.y, size.z, fmt->name);

	display = get_display();

	/* get an appropriate EGL frame buffer configuration */
	ECHK(eglChooseConfig(display, config_attribute_list, &config, 1, &num_config));
	DEBUG_MSG("num_config: %d", num_config);

	/* create an EGL rendering context */
	ECHK(context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribute_list));

	EGLSurface surface = make_window(display, config, 1920, 1080);

	ECHK(eglQuerySurface(display, surface, EGL_WIDTH, &screen_width));
	ECHK(eglQuerySurface(display, surface, EGL_HEIGHT, &screen_height));

	/* connect the context to the surface */
	ECHK(eglMakeCurrent(display, surface, surface, context));

	setup_gl();

	draw_and_check_quads();

	ECHK(eglSwapBuffers(display, surface));
	glFlush();

	ECHK(eglDestroySurface(display, surface));

	ECHK(eglTerminate(display));

	RD_END();
}

int main(int argc, char *argv[])
{
	char *p;

	parse_args(argc, argv);
	print_summary();

	TEST_START();
	TEST(run_texturator());
	TEST_END();

	return 0;
}

