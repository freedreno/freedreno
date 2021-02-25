/*
 * Copyright (c) 2011-2012 Luc Verhaegen <libv@codethink.co.uk>
 * Copyright (c) 2012 Jonathan Maw <jonathan.maw@codethink.co.uk>
 * Copyright (c) 2012 Rob Clark <robdclark@gmail.com>
 * Copyright (c) 2014 Ilia Mirkin <imirkin@alum.mit.edu>
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

/* Code based on cube_textured test from lima driver project (Jonathan Maw)
 * adapted to the logging that I use..
 */

#include <GLES3/gl32.h>
#include "test-util-3d.h"

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

static EGLDisplay display;
static EGLConfig config;
static EGLint num_config;
static EGLContext context;
static GLuint program[4];

#define BLUE	0
#define GREEN	1
#define RED	2
#define YELLOW	3

const char *vertex_shader_source =
		"#version 320 es                                                \n"
		"layout (location = 0) in vec3 aPos;                            \n"
		"                                                               \n"
		"layout (std140) uniform Matrices                               \n"
		"{                                                              \n"
		"    mat4 projection;                                           \n"
		"    mat4 view;                                                 \n"
		"};                                                             \n"
		"uniform mat4 model;                                            \n"
		"                                                               \n"
		"void main()                                                    \n"
		"{                                                              \n"
		"    gl_Position = projection * view * model * vec4(aPos, 1.0); \n"
		"}";

const char *fragment_shader_source_blue =
		"#version 320 es                           \n"
		"precision mediump float;                  \n"
		"out vec4 FragColor;                       \n"
		"                                          \n"
		"void main()                               \n"
		"{                                         \n"
		"    FragColor = vec4(0.0, 0.0, 1.0, 1.0); \n"
		"}";

const char *fragment_shader_source_green =
		"#version 320 es                           \n"
		"precision mediump float;                  \n"
		"out vec4 FragColor;                       \n"
		"                                          \n"
		"void main()                               \n"
		"{                                         \n"
		"    FragColor = vec4(0.0, 1.0, 0.0, 1.0); \n"
		"}";

const char *fragment_shader_source_red =
		"#version 320 es                           \n"
		"precision mediump float;                  \n"
		"out vec4 FragColor;                       \n"
		"                                          \n"
		"void main()                               \n"
		"{                                         \n"
		"    FragColor = vec4(1.0, 0.0, 0.0, 1.0); \n"
		"}";

const char *fragment_shader_source_yellow =
		"#version 320 es                           \n"
		"precision mediump float;                  \n"
		"out vec4 FragColor;                       \n"
		"                                          \n"
		"void main()                               \n"
		"{                                         \n"
		"    FragColor = vec4(1.0, 1.0, 0.0, 1.0); \n"
		"}";

static void test_draw(void)
{
	GLint width, height, ret, i;
	GLfloat vVertices[] = {
		-0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f, -0.5f,  
		-0.5f,  0.5f, -0.5f, 
		-0.5f, -0.5f, -0.5f, 

		-0.5f, -0.5f,  0.5f, 
		 0.5f, -0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f, 
		-0.5f, -0.5f,  0.5f, 

		-0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f, -0.5f, 
		-0.5f, -0.5f, -0.5f, 
		-0.5f, -0.5f, -0.5f, 
		-0.5f, -0.5f,  0.5f, 
		-0.5f,  0.5f,  0.5f, 

		0.5f,  0.5f,  0.5f,  
		0.5f,  0.5f, -0.5f,  
		0.5f, -0.5f, -0.5f,  
		0.5f, -0.5f, -0.5f,  
		0.5f, -0.5f,  0.5f,  
		0.5f,  0.5f,  0.5f,  

		-0.5f, -0.5f, -0.5f, 
		 0.5f, -0.5f, -0.5f,  
		 0.5f, -0.5f,  0.5f,  
		 0.5f, -0.5f,  0.5f,  
		-0.5f, -0.5f,  0.5f, 
		-0.5f, -0.5f, -0.5f, 

		-0.5f,  0.5f, -0.5f, 
		 0.5f,  0.5f, -0.5f,  
		 0.5f,  0.5f,  0.5f,  
		 0.5f,  0.5f,  0.5f,  
		-0.5f,  0.5f,  0.5f, 
		-0.5f,  0.5f, -0.5f,
	};
	GLuint vbo, vao;

	EGLSurface surface;

	RD_START("draw-ubo", "");

	display = get_display();

	/* get an appropriate EGL frame buffer configuration */
	ECHK(eglChooseConfig(display, config_attribute_list, &config, 1, &num_config));
	DEBUG_MSG("num_config: %d", num_config);

	/* create an EGL rendering context */
	ECHK(context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attribute_list));

	surface = make_window(display, config, 255, 255);

	ECHK(eglQuerySurface(display, surface, EGL_WIDTH, &width));
	ECHK(eglQuerySurface(display, surface, EGL_HEIGHT, &height));

	DEBUG_MSG("Buffer: %dx%d", width, height);

	/* connect the context to the surface */
	ECHK(eglMakeCurrent(display, surface, surface, context));

	printf("EGL Version %s\n", eglQueryString(display, EGL_VERSION));
	printf("EGL Vendor %s\n", eglQueryString(display, EGL_VENDOR));
	printf("EGL Extensions %s\n", eglQueryString(display, EGL_EXTENSIONS));
	printf("GL Version %s\n", glGetString(GL_VERSION));
	printf("GL extensions: %s\n", glGetString(GL_EXTENSIONS));

	program[BLUE] = get_program(vertex_shader_source, fragment_shader_source_blue);
	link_program(program[BLUE]);

	program[GREEN] = get_program(vertex_shader_source, fragment_shader_source_green);
	link_program(program[GREEN]);

	program[RED] = get_program(vertex_shader_source, fragment_shader_source_red);
	link_program(program[RED]);

	program[YELLOW] = get_program(vertex_shader_source, fragment_shader_source_yellow);
	link_program(program[YELLOW]);

	GCHK(glGenVertexArrays(1, &vao));
	GCHK(glBindVertexArray(vao));

	GCHK(glGenBuffers(1, &vbo));
	GCHK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GCHK(glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW));

	GCHK(glEnableVertexAttribArray(0));
	GCHK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL));


	/* ubo setup */
	unsigned int uniformBlockIndexBlue = glGetUniformBlockIndex(program[BLUE], "Matrices");
	unsigned int uniformBlockIndexGreen = glGetUniformBlockIndex(program[GREEN], "Matrices");
	unsigned int uniformBlockIndexRed = glGetUniformBlockIndex(program[RED], "Matrices");
	unsigned int uniformBlockIndexYellow = glGetUniformBlockIndex(program[YELLOW], "Matrices");

	GCHK(glUniformBlockBinding(program[BLUE], uniformBlockIndexBlue, 0));
	GCHK(glUniformBlockBinding(program[GREEN], uniformBlockIndexGreen, 0));
	GCHK(glUniformBlockBinding(program[RED], uniformBlockIndexRed, 0));
	GCHK(glUniformBlockBinding(program[YELLOW], uniformBlockIndexYellow, 0));

	unsigned int uboMatrices;
	GCHK(glGenBuffers(1, &uboMatrices));
	GCHK(glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices));
	GCHK(glBufferData(GL_UNIFORM_BUFFER, 2 * 64, NULL, GL_STATIC_DRAW));
	GCHK(glBindBuffer(GL_UNIFORM_BUFFER, 0));
	// define the range of the buffer that links to a uniform binding point
	GCHK(glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * 64));

	GCHK(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
	GCHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// set the view and projection matrix in the uniform block.
	static const float projection[] = {
		1.792591f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.792591f, 0.0f, 0.0f,
		0.0f, 0.0f, -1.002002f, -1.000000f,
		0.0f, 0.0f, -0.200200f, 0.0f
	};
	GCHK(glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices));
	GCHK(glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, projection));
	GCHK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

	static const float view[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, -3.0f, 1.0f
	};
	GCHK(glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices));
	GCHK(glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, view));
	GCHK(glBindBuffer(GL_UNIFORM_BUFFER, 0));

	// draw 4 cubes 
	// RED
	static const float model_red[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-0.75f, 0.75f, 0.0f, 1.0f
	};
        GCHK(glBindVertexArray(vao));
	GCHK(glUseProgram(program[RED]));
	GCHK(glUniformMatrix4fv(glGetUniformLocation(program[RED], "model"), 1, GL_FALSE, model_red));
	GCHK(glDrawArrays(GL_TRIANGLES, 0, 36));

	// GREEN
	static const float model_green[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.75f, 0.75f, 0.0f, 1.0f
	};
	glUseProgram(program[GREEN]);
	glUniformMatrix4fv(glGetUniformLocation(program[GREEN], "model"), 1, GL_FALSE, model_green);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// YELLOW
	static const float model_yellow[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		-0.75f, -0.75f, 0.0f, 1.0f
	};
	glUseProgram(program[YELLOW]);
	glUniformMatrix4fv(glGetUniformLocation(program[YELLOW], "model"), 1, GL_FALSE, model_yellow);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	// BLUE
	static const float model_blue[] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.75f, -0.75f, 0.0f, 1.0f
	};
	glUseProgram(program[BLUE]);
	glUniformMatrix4fv(glGetUniformLocation(program[BLUE], "model"), 1, GL_FALSE, model_blue);
	glDrawArrays(GL_TRIANGLES, 0, 36);


	ECHK(eglSwapBuffers(display, surface));
	GCHK(glFlush());

	ECHK(eglDestroySurface(display, surface));
	ECHK(eglTerminate(display));

	RD_END();
}

int main(int argc, char *argv[])
{
	int i, j;

	TEST_START();
	TEST(test_draw());
	TEST_END();

	return 0;
}
