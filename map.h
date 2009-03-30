#ifndef _MAP_H_
#define _MAP_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include <string>

using namespace std;

GLfloat *rotate_map(GLfloat *map, int w, int h, GLfloat angle);

void to_file(GLfloat *heightmap, int w, int h, string file);
void rgb_to_file(GLfloat *heightmap, int w, int h, string file);

#endif
