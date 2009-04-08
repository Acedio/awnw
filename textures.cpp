#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <cstdlib>
#include <cmath>

using namespace std;

#include "textures.h"
#include "perlin.h"
#include "map.h"

void cloudify(GLfloat *map, int w, int h, GLfloat cover, GLfloat sharpness){
	int y;
	for(y = 0; y < h; y++){
		int x;
		for(x = 0; x < w; x++){
			GLfloat c = map[y*w+x] - cover;
			if(c < 0){
				map[y*w+x] = 0;
			} else {
				map[y*w+x] = 1 - pow(sharpness,c);
			}
		}
	}
}

GLuint make_cloud_texture(){
	//this works well for 256x256 clouds
	int power = 8;
	int size = pow((GLfloat)2,power);
	GLfloat *perlin = perlin_noise(power,power,.5,0,5);

	cloudify(perlin, size, size, .25, .01);

	GLfloat *tex = new GLfloat[size*size*3];
	int y;
	for(y = 0; y < size; y++){
		int x;
		for(x = 0; x < size; x++){
			tex[y*size*3+x*3+0] = perlin[y*size+x];
			tex[y*size*3+x*3+1] = .7+perlin[y*size+x]*.3;
			tex[y*size*3+x*3+2] = .8+perlin[y*size+x]*.2;
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

GLuint make_grass_texture(){
	int power = 8;
	int size = pow((GLfloat)2,power);

	GLfloat *green = perlin_noise(power,power,0.75,2,power);
	GLfloat *yellow = perlin_noise(power,power,0.5,4,power);
	GLfloat *blades_nostretch = perlin_noise(power,power-2,1,0,power-2);
	GLfloat *blades_norotate = smooth_stretch_map(blades_nostretch,size,size>>2,1,4);
	delete[] blades_nostretch;
	GLfloat *blades = rotate_map(blades_norotate,size,size,3.14159/2);
	delete[] blades_norotate;
	GLfloat *tex = new GLfloat[size*size*3];
	for(int y = 0; y < size; y++){
		for(int x = 0; x < size; x++){
			tex[y*size*3+x*3+0] = 0;
			tex[y*size*3+x*3+1] = (green[y*size+x]/2+yellow[y*size+x]/2)*blades[y*size+x];
			tex[y*size*3+x*3+2] = (yellow[y*size+x]/2)*blades[y*size+x]*.6;
		}
	}
	delete[] green;
	delete[] yellow;
	delete[] blades;

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

	delete[] tex;

	return texture;
}

GLuint make_sand_texture(){
	int power = 8;
	int size = pow((GLfloat)2,power);

	GLfloat *dirt = perlin_noise(power, power, .5, 0, power);
	GLfloat *sand = perlin_noise(power, power, .75, 0, power-2);
	GLfloat *tex = new GLfloat[size*size*3];
	for(int y = 0; y < size; y++){
		for(int x = 0; x < size; x++){
			tex[y*size*3+x*3+0] = (dirt[y*size+x]*.5+.5)*sand[y*size+x]*.3+.7;
			tex[y*size*3+x*3+1] = (dirt[y*size+x]*.5+.5)*sand[y*size+x]*.3+.7;
			tex[y*size*3+x*3+2] = (dirt[y*size+x]*.5+.3)*sand[y*size+x]*.2+.5;
		}
	}
	delete[] dirt;
	delete[] sand;

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

	delete[] tex;

	return texture;
}

GLuint make_rock_texture(){
	int power = 8;
	int size = pow((GLfloat)2,power);

	GLfloat *rock = perlin_noise(power, power, .75, 2, power);
	GLfloat *tex = new GLfloat[size*size*3];
	for(int y = 0; y < size; y++){
		for(int x = 0; x < size; x++){
			tex[y*size*3+x*3+0] = .3+rock[y*size+x]*.7;
			tex[y*size*3+x*3+1] = .3+rock[y*size+x]*.7;
			tex[y*size*3+x*3+2] = .3+rock[y*size+x]*.6;
		}
	}
	delete[] rock;
	rgb_to_file(tex,size,size,"lol.ppm");

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

	delete[] tex;

	return texture;
}

GLuint make_tree_texture(){
	int power = 8;
	int size = pow((GLfloat)2,power);
	GLfloat *tree_nostretch = new GLfloat[size*size/16];
	for(int y = 0; y < size/4; y++){
		for(int x = 0; x < size/4; x++){
			GLfloat d = (GLfloat)(x-size/8)/(GLfloat)(size/4)*2;
			tree_nostretch[y*size/4+x] = -1.7*pow((float)abs(d),(float)1.3)+4*(GLfloat)y/(GLfloat)size;
			if(tree_nostretch[y*size/4+x] < 0){
				tree_nostretch[y*size/4+x] = 0;
			} else if(tree_nostretch[y*size/4+x] > 1){
				tree_nostretch[y*size/4+x] = 1;
			}
		}
	}
	to_file(tree_nostretch,size/4,size/4,"tree.pgm");
	GLfloat *tree = smooth_stretch_map(tree_nostretch, size/4, size/4, 4, 4);
	delete[] tree_nostretch;
	GLfloat *foliage_nostretch = perlin_noise(power-2, power, .75, 1, power-2);
	GLfloat *foliage = smooth_stretch_map(foliage_nostretch, size/4, size, 4, 1);
	delete[] foliage_nostretch;
	GLfloat *tex = new GLfloat[size*size*4];
	for(int y = 0; y < size; y++){
		for(int x = 0; x < size; x++){
			tex[y*size*4+x*4+0] = .1+.1*(1-.8*tree[y*size+x])*foliage[y*size+x];
			tex[y*size*4+x*4+1] = .2+.4*(1-.8*tree[y*size+x])*foliage[y*size+x];
			tex[y*size*4+x*4+2] = .2*(1-.8*tree[y*size+x])*foliage[y*size+x];
			if(tree[y*size+x] > 0){
				tex[y*size*4+x*4+3] = 1; 
			} else {
				tex[y*size*4+x*4+3] = 0;
			}
		}
	}
	delete[] tree;
	delete[] foliage;

	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGBA,size,size,GL_RGBA,GL_FLOAT,tex);

	delete[] tex;

	return texture;
}

GLfloat *range_fade(GLfloat *heightmap, int w, int h, GLfloat fadein_low, GLfloat fadein_high, GLfloat fadeout_low, GLfloat fadeout_high){
	GLfloat *fademap = new GLfloat[w*h];
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			GLfloat height = heightmap[y*w+x];
			if(height < fadein_low){
				fademap[y*w+x] = 0;
			} else if(height < fadein_high){
				fademap[y*w+x] = (height - fadein_low)/(fadein_high - fadein_low);
			} else if(height < fadeout_low){
				fademap[y*w+x] = 1;
			} else if(height < fadeout_high){
				fademap[y*w+x] = 1 - (height - fadeout_low)/(fadeout_high-fadeout_low);
			} else {
				fademap[y*w+x] = 0;
			}
		}
	}
	return fademap;
}

GLuint range_fade_texture(GLfloat *heightmap, int w, int h, GLfloat fadein_low, GLfloat fadein_high, GLfloat fadeout_low, GLfloat fadeout_high){
	GLfloat *fademap = range_fade(heightmap, w, h, fadein_low, fadein_high, fadeout_low, fadeout_high);

	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_ALPHA,w,h,GL_ALPHA,GL_FLOAT,fademap);

	delete[] fademap;

	return texture;
}
