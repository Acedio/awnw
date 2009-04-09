#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "terramap.h"
#include "textures.h"

GLfloat rand_float(){
	return 2*((GLfloat)rand()/(GLfloat)RAND_MAX) - 1;
}

void display_heightmap(GLfloat *heightmap, int w, int h){
// used early on for testing
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			cout << heightmap[y*w+x] << "\t";
		}
		cout << endl;
	}
}

GLfloat *make_terramap(int power, GLfloat displace){
// this is the old way of creating my heightmaps. faster than perlin, but really better for mountainous regions and wasn't what I was going for
	int size = (int)pow((GLfloat)2,power);
	GLfloat *heightmap = new GLfloat[size*size];
	int level;
	for(level = 0; level < power; level++){
		int inc = size/(int)pow((GLfloat)2,level);
		int start = inc/2;
		int x;
		// squares to diamonds
		for(x = 0; x < size; x += inc){
			int y;
			for(y = 0; y < size; y += inc){
				heightmap[(start+y)*size+start+x] = (heightmap[y*size+x] + heightmap[y*size+(x+inc)%size] + heightmap[((y+inc)%size)*size+x] + heightmap[((y+inc)%size)*size+(x+inc)%size])/4.0 + rand_float()*(GLfloat)inc*displace;
			}
		}
		// diamonds to squares
		for(x = start; x < size; x += inc){
			int y;
			for(y = 0; y < size; y += inc){
				heightmap[y*size+x] = (heightmap[((y+inc/2)%size)*size+x] + heightmap[y*size+(x+inc/2)%size] + heightmap[((size+y-inc/2)%size)*size+x] + heightmap[y*size+(size+x-inc/2)%size])/4.0 + rand_float()*(GLfloat)inc*displace;
			}
		}
		for(x = 0; x < size; x += inc){
			int y;
			for(y = start; y < size; y += inc){
				heightmap[y*size+x] = (heightmap[((y+inc/2)%size)*size+x] + heightmap[y*size+(x+inc/2)%size] + heightmap[((size+y-inc/2)%size)*size+x] + heightmap[y*size+(size+x-inc/2)%size])/4.0 + rand_float()*(GLfloat)inc*displace;
			}
		}
	}
	return heightmap;
}

void draw_heightmap_vector(GLfloat *heightmap, int w, int h){
// awesome old-school-esque rendering of the heightmap
	int x;
	glColor3f(0,.1,0); // solid first
	glDisable(GL_LIGHTING);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2,2);
	glBegin(GL_QUADS);
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			glVertex3f((GLfloat)x,heightmap[(y%h)*w+x%w],(GLfloat)y);
			glVertex3f((GLfloat)x,heightmap[((y+1)%h)*w+x%w],(GLfloat)(y+1));
			glVertex3f((GLfloat)(x+1),heightmap[((y+1)%h)*w+(x+1)%w],(GLfloat)(y+1));
			glVertex3f((GLfloat)(x+1),heightmap[(y%h)*w+(x+1)%w],(GLfloat)y);
		}
	}
	glEnd();
	glColor3f(0,1,0); // now for vector lines
	glPolygonOffset(1,1);
	glBegin(GL_LINES);
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			glVertex3f((GLfloat)x,heightmap[(y%h)*w+x%w],(GLfloat)y);
			glVertex3f((GLfloat)(x+1),heightmap[(y%h)*w+(x+1)%w],(GLfloat)y);

			glVertex3f((GLfloat)x,heightmap[(y%h)*w+x%w],(GLfloat)y);
			glVertex3f((GLfloat)x,heightmap[((y+1)%h)*w+x%w],(GLfloat)(y+1));
		}
	}
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_LIGHTING);
}

inline void terrain_color(GLfloat **heightmap, int x, int y){//CHANGE ME TO SINGLE DIM ARR
// Not used anymore. I'd cut it but I don't feel like it ;D
	return;
	if(heightmap[y][x] < 1){
		glColor3f(0,0,.3+sqrt(-1/(heightmap[y][x]-2))*.7);
	} else if(heightmap[y][x] < 3){
		glColor3f(.8,.7,.2);
	} else if(heightmap[y][x] < 6){
		glColor3f(0,.5+.3*(heightmap[y][x]-3)/3,0);
	} else if(heightmap[y][x] < 10){
		glColor3f(.5+.3*(heightmap[y][x]-6)/4,.6+.3*(heightmap[y][x]-6)/4,.4+.3*(heightmap[y][x]-6)/4);
	} else {
		glColor3f(1,1,1);
	}
}

inline GLuint terrain_texture(GLfloat **heightmap, int x, int y, GLuint textures[TEXTURE_COUNT]){//CHANGE ME TO SINGLE DIM ARR
// see above. not used
	if(heightmap[y][x] < 1){
		return textures[TEXTURE_SAND];
	} else if(heightmap[y][x] < 3){
		return textures[TEXTURE_SAND];
	} else if(heightmap[y][x] < 6){
		return textures[TEXTURE_GRASS];
	} else if(heightmap[y][x] < 10){
		return textures[TEXTURE_GRASS];
	} else {
		return textures[TEXTURE_GRASS];
	}
}

void draw_heightmap_texture(GLfloat *heightmap, GLfloat **normalmap, GLuint textures[TEXTURE_COUNT], int max_textures, int w, int h){
// Here's our nifty optimized map display. Very tasty with a bit of multitexturing.
	glColor3f(1,1,1);

	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
	glEnable(GL_LIGHT0);

	glActiveTexture(GL_TEXTURE0); // we always have atleast one texture stage
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_GRASS]);
	
	if(max_textures > 2){ // enough for sand layer?
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_GRASS_ALPHA]);

		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SAND]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE2);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE1);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);

		if(max_textures > 4){ //enough for rock layer?
			glActiveTexture(GL_TEXTURE3);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_ROCK_ALPHA]);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_INTERPOLATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE4);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PREVIOUS);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE2_RGB, GL_TEXTURE3);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB, GL_SRC_ALPHA);

			glActiveTexture(GL_TEXTURE4);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_ROCK]);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
			glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR);
			glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		}
	}

	glBegin(GL_TRIANGLE_STRIP);
	for(int y = 0; y < h; y++){
		GLfloat fy = (GLfloat)(y)/32.0;
		GLfloat fy1 = (GLfloat)(1+y)/32.0;
		for(int x = 0; x <= w; x++){
			GLfloat fx = (GLfloat)(x)/32.0;

			glMultiTexCoord2f(GL_TEXTURE0,fx,fy);
			if(max_textures > 2){
				glMultiTexCoord2f(GL_TEXTURE1,(GLfloat)x/(GLfloat)w,(GLfloat)y/(GLfloat)h);
				glMultiTexCoord2f(GL_TEXTURE2,fx,fy);
				if(max_textures > 4){
					glMultiTexCoord2f(GL_TEXTURE3,(GLfloat)x/(GLfloat)w,(GLfloat)y/(GLfloat)h);
					glMultiTexCoord2f(GL_TEXTURE4,fx,fy);
				}
			}
			glNormal3fv(normalmap[(y%h)*w+x%w]);
			glVertex3f((GLfloat)x,heightmap[(y%h)*w+x%w],(GLfloat)y);

			glMultiTexCoord2f(GL_TEXTURE0,fx,fy1);
			if(max_textures > 2){
				glMultiTexCoord2f(GL_TEXTURE1,(GLfloat)x/(GLfloat)w,(GLfloat)(y+1)/(GLfloat)h);
				glMultiTexCoord2f(GL_TEXTURE2,fx,fy1);
				if(max_textures > 4){
					glMultiTexCoord2f(GL_TEXTURE3,(GLfloat)x/(GLfloat)w,(GLfloat)(y+1)/(GLfloat)h);
					glMultiTexCoord2f(GL_TEXTURE4,fx,fy1);
				}
			}
			glNormal3fv(normalmap[((y+1)%h)*w+x%w]);
			glVertex3f((GLfloat)x,heightmap[((y+1)%h)*w+x%w],(GLfloat)(y+1));
		}
		glMultiTexCoord2f(GL_TEXTURE0,(GLfloat)w/(GLfloat)32,fy1);
		if(max_textures > 2){
			glMultiTexCoord2f(GL_TEXTURE1,(GLfloat)1,(GLfloat)(y+1)/(GLfloat)h);
			glMultiTexCoord2f(GL_TEXTURE2,(GLfloat)w/(GLfloat)32,fy1);
			if(max_textures > 4){
				glMultiTexCoord2f(GL_TEXTURE3,(GLfloat)1,(GLfloat)(y+1)/(GLfloat)h);
				glMultiTexCoord2f(GL_TEXTURE4,(GLfloat)w/(GLfloat)32,fy1);
			}
		}
		glNormal3fv(normalmap[(y+1)%h+0]);
		glVertex3f((GLfloat)w,heightmap[((y+1)%h)*w+0],(GLfloat)(y+1));

		glMultiTexCoord2f(GL_TEXTURE0,0,fy1);
		if(max_textures > 2){
			glMultiTexCoord2f(GL_TEXTURE1,0,fy1);
			glMultiTexCoord2f(GL_TEXTURE2,0,(GLfloat)(y+1)/(GLfloat)h);
			if(max_textures > 4){
				glMultiTexCoord2f(GL_TEXTURE3,0,(GLfloat)(y+1)/(GLfloat)h);
				glMultiTexCoord2f(GL_TEXTURE4,0,fy1);
			}
		}
		glNormal3fv(normalmap[((y+1)%h)*w+0]);
		glVertex3f((GLfloat)0,heightmap[((y+1)%h)*w+0],(GLfloat)(y+1));
	}
	glEnd();

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);
}

GLfloat **make_normalmap(GLfloat *heightmap, int w, int h){
// really cheesy method of generating normal maps, it's using surface normals currently. Should use the average of the 6 surface normals instead.
	GLfloat **normalmap = new GLfloat*[w*h];
	int y;
	for(y = 0; y < h; y++){
		int x;
		for(x = 0; x < w; x++){
			normalmap[y*w+x] = new GLfloat[3];
			GLfloat i,j,k;
			i = -(heightmap[y*w+(x+1)%w]-heightmap[y*w+x]);
			j = 1;
			k = -(heightmap[((y+1)%h)*w+x]-heightmap[y*w+x]);
			GLfloat m = sqrt(i*i + j*j + k*k);
			normalmap[y*w+x][0] = i/m;
			normalmap[y*w+x][1] = j/m;
			normalmap[y*w+x][2] = k/m;
		}
	}
	return normalmap;
}

void oceanify(GLfloat *heightmap, int w, int h, GLfloat min){
// clamps the lower end of height values to [min]
	int x;
	bool doShift = false;
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			if(heightmap[y*w+x] < 0){
				doShift = true;
				heightmap[y*w+x] = (-1.0)/(heightmap[y*w+x]-1) - 1;
			}
		}
	}
	if(doShift){
		for(x = 0; x < w; x++){
			int y;
			for(y = 0; y < h; y++){
				heightmap[y*w+x] += 1;
			}
		}
	}
}

void hillify(GLfloat *heightmap, int w, int h, GLfloat flatness){
// makes heightmap taller or shorter, yet retains slope extremes more or less
	oceanify(heightmap, w, h, 0.1); // no negatives, yo!
	int x;
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			heightmap[y*w+x] = (GLfloat)pow((GLfloat)heightmap[y*w+x],(GLfloat)flatness);
		}
	}
}

void smoothify(GLfloat *heightmap, int w, int h, GLfloat inertia){
// fun smoothing (really blurring, actually) function. fun to watch.
	if(inertia > 1) inertia = 1;
	if(inertia < 0) inertia = 0;
	int x;
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			heightmap[y*w+x] = (heightmap[y*w+x]*inertia + (1-inertia)*(heightmap[((h+y-1)%h)*w+x] + heightmap[y*w+(w+x-1)%w] + heightmap[((y+1)%h)*w+x] + heightmap[y*w+(x+1)%w])/4.0);
		}
	}
}

GLfloat *normalize(GLfloat *heightmap, int w, int h, GLfloat depth){
// scales a heightmap to fit between [0,1]
	GLfloat min, max;
	min = max = heightmap[0];
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			if(heightmap[y*w+x] < min){
				min = heightmap[y*w+x];
			} else if(heightmap[y*w+x] > max){
				max = heightmap[y*w+x];
			}
		}
	}
	GLfloat *normalized = new GLfloat[w*h];
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			normalized[y*w+x] = depth * (heightmap[y*w+x] - min) / (max - min);
		}
	}
	return normalized;
}

void out_to_file(GLfloat *heightmap, int w, int h, string file){
// actually, this is basically the same as map.cpp's to_file function. ppms and pgms are awesome for this.
	if(w > 0 && h > 0){
		ofstream of(file.c_str());
		of << "P2 " << w << " " << h << " 255\n";
		int y,x;
		GLfloat max = heightmap[0];
		GLfloat min = max;
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				if(heightmap[y*w+x] > max){
					max = heightmap[y*w+x];
				}
				if(heightmap[y*w+x] < min){
					min = heightmap[y*w+x];
				}
			}
		}
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				of << (int)(255*(heightmap[y*w+x]-min)/(max-min)) << " ";
			}
			of << "\n";
		}
		of.close();
	}
}
