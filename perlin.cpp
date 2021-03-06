#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <cstdlib>
#include <cmath>
#include <ctime>

using namespace std;

#include "perlin.h"

GLfloat noise(int x){
// a simple rng. Useful for recreating our trees with the same seed and all.
	x = (x<<13) ^ x;
	return (GLfloat)(((x * (x * x * 15731 + 789221) + 1376312589) & 0x7FFFFFFF) / 1073741824.0)/2.0;
}

GLfloat smoothstep(GLfloat x){
// nice, easy interpolation function
	return x * x * (3 - 2 * x);
}

GLfloat *create_noise_map(int w, int h){
// returns a map w*h in size filled with noise
	GLfloat *map = new GLfloat[w*h];
	int y;
	for(y = 0; y < h; y++){
		int x;
		for(x = 0; x < w; x++){
			map[y*w+x] = (GLfloat)rand()/(GLfloat)RAND_MAX;
		}
	}
	return map;
}

GLfloat *linear_stretch_map(GLfloat *map, int w, int h, int x_scale, int y_scale){
// stretches a map according to scales. this version is linear (quick but UGLY).
	int x, y;
	GLfloat *stretched = new GLfloat[w*x_scale*h*y_scale];
	for(y = 0; y < h; y++){
		for(x = 0; x < w; x++){
			int xs;
			GLfloat i = map[y*w+x];
			GLfloat f = map[y*w+((x+1)%w)];
			GLfloat step = (f-i)/(GLfloat)x_scale;
			for(xs = 0; xs < x_scale; xs++){
				stretched[y_scale*y*x_scale*w+x_scale*x+xs] = i;
				i += step;
			}
		}
	}
	for(x = 0; x < w*x_scale; x++){
		for(y = 0; y < h; y++){
			int ys;
			GLfloat i = stretched[y*y_scale*w*x_scale+x];
			GLfloat f = stretched[((y+1)%h)*y_scale*w*x_scale+x];
			GLfloat step = (f-i)/(GLfloat)y_scale;
			for(ys = 0; ys < y_scale; ys++){
				stretched[(y*y_scale+ys)*w*x_scale+x] = i;
				i += step;
			}
		}
	}
	return stretched;
}

GLfloat *smooth_stretch_map(GLfloat *map, int w, int h, int x_scale, int y_scale){
// stretches a map according to scales. this version is smoothstepped (a little slower but much nicer looking).
	int x, y;
	GLfloat *stretched = new GLfloat[h*y_scale*w*x_scale];
	for(y = 0; y < h; y++){
		for(x = 0; x < w; x++){
			GLfloat i = map[y*w+x];
			GLfloat f = map[y*w+((x+1)%w)];
			GLfloat step = 1/(GLfloat)x_scale;
			int xs;
			for(xs = 0; xs < x_scale; xs++){
				stretched[y_scale*y*w*x_scale+x_scale*x+xs] = i+(f-i)*smoothstep(step*(GLfloat)xs);
			}
		}
	}
	for(x = 0; x < w*x_scale; x++){
		for(y = 0; y < h; y++){
			GLfloat i = stretched[y*y_scale*w*x_scale+x];
			GLfloat f = stretched[((y+1)%h)*y_scale*w*x_scale+x];
			GLfloat step = 1/(GLfloat)y_scale;
			int ys;
			for(ys = 0; ys < y_scale; ys++){
				stretched[(y*y_scale+ys)*w*x_scale+x] = i+(f-i)*smoothstep(step*(GLfloat)ys);
			}
		}
	}
	return stretched;
}

GLfloat *perlin_noise(int x_pow, int y_pow, GLfloat persistence, int start, int end){
// the awesomest 2d noise function ever. used for EVERYTHING in this program pretty much.
	if(end > x_pow || end > y_pow || start < 0 || end < start){
		return NULL;
	}
	int w, h;
	w = (int)pow((GLfloat)2,x_pow);
	h = (int)pow((GLfloat)2,y_pow);
	GLfloat *map = new GLfloat[w*h];
	for(int i = 0; i < w*h; i++){
		map[i] = 0;
	}
	GLfloat p = 1, ptot = 0;
	for(int i = (int)pow((GLfloat)2,end); i >= (int)pow((GLfloat)2,start); i/=2){
		GLfloat *noise_map = create_noise_map(w/i,h/i);
		GLfloat *temp = smooth_stretch_map(noise_map,w/i,h/i,i,i);
		delete[] noise_map;
		int y;
		for(y = 0; y < h; y++){
			int x;
			for(x = 0; x < w; x++){
				map[y*w+x] += temp[y*w+x]*p;
			}
		}
		delete[] temp;
		ptot += p;
		p *= persistence;
	}
	int y;
	for(y = 0; y < h; y++){
		int x;
		for(x = 0; x < w; x++){
			map[y*w+x] /= ptot;
		}
	}
	return map;
}
