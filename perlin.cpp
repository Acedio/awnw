#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <cstdlib>
#include <cmath>
#include <string>
#include <fstream>

using namespace std;

#include "perlin.h"

inline GLfloat smoothstep(GLfloat x){
	return x * x * (3 - 2 * x);
}

GLfloat **create_noise_map(int w, int h){
	GLfloat **map = new GLfloat*[h];
	int y;
	for(y = 0; y < h; y++){
		map[y] = new GLfloat[w];
		int x;
		for(x = 0; x < w; x++){
			map[y][x] = (GLfloat)rand()/(GLfloat)RAND_MAX;
		}
	}
	return map;
}

GLfloat **linear_stretch_map(GLfloat **map, int w, int h, int x_scale, int y_scale){
	int x, y;
	GLfloat **stretched = new GLfloat*[h*y_scale];
	for(y = 0; y < h; y++){
		int ys;
		for(ys = 0; ys < y_scale; ys++){
			stretched[y_scale*y+ys] = new GLfloat[w*x_scale];
		}
		for(x = 0; x < w; x++){
			int xs;
			GLfloat i = map[y][x];
			GLfloat f = map[y][(x+1)%w];
			GLfloat step = (f-i)/(GLfloat)x_scale;
			for(xs = 0; xs < x_scale; xs++){
				stretched[y_scale*y][x_scale*x+xs] = i;
				i += step;
			}
		}
	}
	for(x = 0; x < w*x_scale; x++){
		for(y = 0; y < h; y++){
			int ys;
			GLfloat i = stretched[y*y_scale][x];
			GLfloat f = stretched[((y+1)%h)*y_scale][x];
			GLfloat step = (f-i)/(GLfloat)y_scale;
			for(ys = 0; ys < y_scale; ys++){
				stretched[y*y_scale+ys][x] = i;
				i += step;
			}
		}
	}
	return stretched;
}

GLfloat **smooth_stretch_map(GLfloat **map, int w, int h, int x_scale, int y_scale){
	int x, y;
	GLfloat **stretched = new GLfloat*[h*y_scale];
	for(y = 0; y < h; y++){
		int ys;
		for(ys = 0; ys < y_scale; ys++){
			stretched[y_scale*y+ys] = new GLfloat[w*x_scale];
		}
		for(x = 0; x < w; x++){
			GLfloat i = map[y][x];
			GLfloat f = map[y][(x+1)%w];
			GLfloat step = 1/(GLfloat)x_scale;
			int xs;
			for(xs = 0; xs < x_scale; xs++){
				stretched[y_scale*y][x_scale*x+xs] = i+(f-i)*smoothstep(step*(GLfloat)xs);
			}
		}
	}
	for(x = 0; x < w*x_scale; x++){
		for(y = 0; y < h; y++){
			GLfloat i = stretched[y*y_scale][x];
			GLfloat f = stretched[((y+1)%h)*y_scale][x];
			GLfloat step = 1/(GLfloat)y_scale;
			int ys;
			for(ys = 0; ys < y_scale; ys++){
				stretched[y*y_scale+ys][x] = i+(f-i)*smoothstep(step*(GLfloat)ys);
			}
		}
	}
	return stretched;
}

GLfloat **perlin_noise(int size, GLfloat persistence, int start, int end){
	if(end > size || start < 0 || end < start){
		return NULL;
	}
	int s;
	s = (int)pow((GLfloat)2,size);
	int i;
	GLfloat **map = new GLfloat*[s];
	for(i = 0; i < s; i++){
		map[i] = new GLfloat[s];
	}
	GLfloat p = 1, ptot = 0;
	for(i = (int)pow((GLfloat)2,end); i >= (int)pow((GLfloat)2,start); i/=2){
		GLfloat **noise_map = create_noise_map(s/i,s/i);
		GLfloat **temp = smooth_stretch_map(noise_map,s/i,s/i,i,i);
		for(int j = 0; j < s/i; j++){
			delete[] noise_map[j];
		}
		delete[] noise_map;
		int y;
		for(y = 0; y < s; y++){
			int x;
			for(x = 0; x < s; x++){
				map[y][x] += temp[y][x]*p;
			}
		}
		for(int j = 0; j < s; j++){
			delete[] temp[j];
		}
		delete[] temp;
		ptot += p;
		p *= persistence;
	}
	int y;
	for(y = 0; y < s; y++){
		int x;
		for(x = 0; x < s; x++){
			map[y][x] /= ptot;
		}
	}
	return map;
}

void cloudify(GLfloat **map, int w, int h, GLfloat cover, GLfloat sharpness){
	int y;
	for(y = 0; y < h; y++){
		int x;
		for(x = 0; x < w; x++){
			GLfloat c = map[y][x] - cover;
			if(c < 0){
				map[y][x] = 0;
			} else {
				map[y][x] = 1 - pow(sharpness,c);
			}
		}
	}
}

void to_file(GLfloat **heightmap, int w, int h, string file){
	if(w > 0 && h > 0){
		ofstream of(file.c_str());
		of << "P2 " << w << " " << h << " 255\n";
		int y,x;
		GLfloat max = 1;
		GLfloat min = 0;
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				of << (int)(255*(heightmap[y][x]-min)/(max-min)) << " ";
			}
			of << "\n";
		}
		of.close();
	}
}

void rgb_to_file(GLfloat *heightmap, int w, int h, string file){
	if(w > 0 && h > 0){
		ofstream of(file.c_str());
		of << "P3 " << w << " " << h << " 255\n";
		int y,x;
		int max = 1;
		int min = 0;
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				of << (int)(255*(heightmap[y*w+x*3+0]-min)/(max-min)) << " " << (int)(255*(heightmap[y*w+x*3+1]-min)/(max-min)) << " " << (int)(255*(heightmap[y*w+x*3+2]-min)/(max-min)) << " ";
			}
			of << "\n";
		}
		of.close();
	}
}

GLuint make_cloud_texture(){
	//this works well for 256x256 clouds
	int power = 8;
	int size = pow((GLfloat)2,power);
	GLfloat **perlin = perlin_noise(power,.5,0,5);

	to_file(perlin, size, size, "perlin.pgm");

	cloudify(perlin, size, size, .5, .01);

	to_file(perlin, size, size, "cloudified.pgm");

	GLfloat *tex = new GLfloat[size*size*3];
	int y;
	for(y = 0; y < size; y++){
		int x;
		for(x = 0; x < size; x++){
			tex[y*size*3+x*3+0] = perlin[y][x];
			tex[y*size*3+x*3+1] = .7+perlin[y][x]*.3;
			tex[y*size*3+x*3+2] = .8+perlin[y][x]*.2;
		}
	}

	rgb_to_file(tex,size,size,"colored.ppm");

	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,size,size,GL_RGB,GL_FLOAT,tex);

	for(int i = 0; i < size; i++){
		delete[] perlin[i];
	}
	delete[] perlin;
	delete[] tex;

	return texture;
}

GLuint make_ground_texture(){
	int power = 8;
	int size = pow((GLfloat)2,power);

	GLfloat **perlin = perlin_noise(power,1.0,0,power);
	GLfloat *tex = new GLfloat[size*size*3];

	int y;
	for(y = 0; y < size; y++){
		int x;
		for(x = 0; x < size; x++){
			tex[y*size*3+x*3+0] = tex[y*size*3+x*3+1] = tex[y*size*3+x*3+2] = perlin[y][x];
		}
	}

	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,size,size,GL_RGB,GL_FLOAT,tex);

	delete[] perlin;
	delete[] tex;

	return texture;
}
