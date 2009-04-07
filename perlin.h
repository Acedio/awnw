#ifndef _PERLIN_H_
#define _PERLIN_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>

GLfloat *create_noise_map(int w, int h);

GLfloat *linear_stretch_map(GLfloat *map, int w, int h, int x_scale, int y_scale);
GLfloat *smooth_stretch_map(GLfloat *map, int w, int h, int x_scale, int y_scale);

GLfloat *perlin_noise(int x_pow, int y_pow, GLfloat persistence, int start, int end);

#endif
