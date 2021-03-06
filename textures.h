#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>

GLuint make_cloud_texture();
GLuint make_grass_texture();
GLuint make_sand_texture();
GLuint make_rock_texture();
GLuint make_tree_texture();

GLfloat *range_fade(GLfloat *heightmap, int w, int h, GLfloat fadein_low, GLfloat fadein_high, GLfloat fadeout_low, GLfloat fadeout_high);
GLuint range_fade_texture(GLfloat *heightmap, int w, int h, GLfloat fadein_low, GLfloat fadein_high, GLfloat fadeout_low, GLfloat fadeout_high);

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
