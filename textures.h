#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

GLuint make_cloud_texture();
GLuint make_ground_texture();
GLuint make_grass_texture();
GLuint make_sand_texture();

void cloudify(GLfloat *map, int w, int h, GLfloat cover, GLfloat sharpness);

const int TEXTURE_DEFAULT = 0;
const int TEXTURE_GRASS = 1;
const int TEXTURE_SAND = 2;
const int TEXTURE_ALPHA = 3;
const int TEXTURE_COUNT = 4;

#endif
