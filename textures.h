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
GLuint make_rock_texture();

void cloudify(GLfloat *map, int w, int h, GLfloat cover, GLfloat sharpness);

const int TEXTURE_NONE = 0;
const int TEXTURE_SAND = 1;
const int TEXTURE_SAND_ALPHA = 2;
const int TEXTURE_GRASS = 3;
const int TEXTURE_GRASS_ALPHA = 4;
const int TEXTURE_ROCK = 5;
const int TEXTURE_ROCK_ALPHA = 6;
const int TEXTURE_COUNT = 7;

#endif
