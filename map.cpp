#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <cstdlib>
#include <cmath>
#include <string>
#include <fstream>

using namespace std;

#include "map.h"

const GLfloat PI = 3.14159;

GLfloat *rotate_map(GLfloat *map, int w, int h, GLfloat angle){
// not really used, actually. These rotated textures don't repeat and I can always fool with the texture matrix to get the right effect
	GLfloat *rotated = new GLfloat[w*h];
	for(int y = 0; y < h; y++){
		GLfloat cy = sin(angle+PI/2)*(GLfloat)y;
		GLfloat cx = cos(angle+PI/2)*(GLfloat)y;
		for(int x = 0; x < w; x++){
			if(cy < 0){
				cy += (GLfloat)h;
			}
			if(cx < 0){
				cx += (GLfloat)w;
			}
			rotated[y*w+x] = map[((int)cy%h)*w+(int)cx%w];
			cy += sin(angle);
			cx += cos(angle);
		}
	}
	return rotated;
}

void to_file(GLfloat *heightmap, int w, int h, string file){
// used for testing the texture functions. this one is for heightmaps
	if(w > 0 && h > 0){
		ofstream of(file.c_str());
		of << "P2 " << w << " " << h << " 255\n";
		int y,x;
		GLfloat max = 1;
		GLfloat min = 0;
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				of << (int)(255*(heightmap[y*w+x]-min)/(max-min)) << " ";
			}
			of << "\n";
		}
		of.close();
	}
}

void rgb_to_file(GLfloat *heightmap, int w, int h, string file){
// used for testing the texture functions. this one is for colormaps
	if(w > 0 && h > 0){
		ofstream of(file.c_str());
		of << "P3 " << w << " " << h << " 255\n";
		int y,x;
		int max = 1;
		int min = 0;
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				of << (int)(255*(heightmap[y*w*3+x*3+0]-min)/(max-min)) << " " << (int)(255*(heightmap[y*w*3+x*3+1]-min)/(max-min)) << " " << (int)(255*(heightmap[y*w*3+x*3+2]-min)/(max-min)) << " ";
			}
			of << "\n";
		}
		of.close();
	}
}
