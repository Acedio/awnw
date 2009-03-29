#ifndef _TERRAMAP_H_
#define _TERRAMAP_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include <string>

using namespace std;

GLfloat **create_heightmap(int w, int h);

GLfloat **make_terramap(int power, GLfloat displace);
GLfloat ***make_normalmap(GLfloat **heightmap, int w, int h);

void display_heightmap(GLfloat **heightmap, int w, int h);

void draw_heightmap_vector(GLfloat **heightmap, int w, int h, GLfloat x_scale, GLfloat y_scale, GLfloat z_scale);
void terrain_color(GLfloat **heightmap, int x, int y);
void draw_heightmap_texture(GLfloat **heightmap, GLfloat ***normalmap, int w, int h, GLfloat x_scale, GLfloat y_scale, GLfloat z_scale);

void oceanify(GLfloat **heightmap, int w, int h, GLfloat min);
void hillify(GLfloat **heightmap, int w, int h, GLfloat flatness);
void smoothify(GLfloat **heightmap, int w, int h, GLfloat inertia);
GLfloat **normalize(GLfloat **heightmap, int w, int h, GLfloat depth);

void out_to_file(GLfloat **heightmap, int w, int h, string file);

#endif
