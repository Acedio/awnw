#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#include <cstdlib>
#include <cmath>

using namespace std;

#include "textures.h"
#include "perlin.h"


GLuint make_cloud_texture(){
	//this works well for 256x256 clouds
	int power = 8;
	int size = pow((GLfloat)2,power);
	GLfloat *perlin = perlin_noise(power,.5,0,5);

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

GLuint make_ground_texture(){
	int power = 8;
	int size = pow((GLfloat)2,power);

	GLfloat *tex = perlin_noise(power,0.75,2,power);

	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_LUMINANCE,size,size,GL_LUMINANCE,GL_FLOAT,tex);

	delete[] tex;

	return texture;
}

GLuint make_grass_texture(){
	int power = 8;
	int size = pow((GLfloat)2,power);

	GLfloat *green = perlin_noise(power,0.75,2,power);
	GLfloat *yellow = perlin_noise(power,0.5,4,power);
	GLfloat *blades_nostretch = perlin_noise(power-2,1,0,power-2);
	GLfloat *blades = smooth_stretch_map(blades_nostretch,size/4,size/4,1,4);
	delete[] blades_nostretch;
	GLfloat *tex = new GLfloat[size*size*3];
	for(int y = 0; y < size; y++){
		for(int x = 0; x < size; x++){
			tex[y*size*3+x*3+0] = 0;
			tex[y*size*3+x*3+1] = (green[y*size+x]/2+yellow[y*size+x]/2)*blades[y*size/4+x%(size/4)];
			tex[y*size*3+x*3+2] = (yellow[y*size+x]/2)*blades[y*size/4+x%(size/4)];
		}
	}
	//rgb_to_file(tex,size,size,"lol.ppm");

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