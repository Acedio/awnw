#ifndef _PERLIN_H_
#define _PERLIN_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

GLfloat **create_noise_map(int w, int h);

GLfloat **linear_stretch_map(GLfloat **map, int w, int h, int x_scale, int y_scale);
GLfloat **smooth_stretch_map(GLfloat **map, int w, int h, int x_scale, int y_scale);

GLfloat **perlin_noise(int size, GLfloat persistence, int start, int end);

void cloudify(GLfloat **map, int w, int h, GLfloat cover, GLfloat sharpness);

GLuint make_cloud_texture();
GLuint make_ground_texture();

#endif
