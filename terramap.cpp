#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

#include "terramap.h"

GLfloat **create_heightmap(int w, int h){
	GLfloat **heightmap = new GLfloat*[h];
	int y;
	for(y = 0; y < h; y++){
		heightmap[y] = new GLfloat[w];
		int x;
		for(x = 0; x < w; x++){
			heightmap[y][x] = 0;
		}
	}
	return heightmap;
}

GLfloat rand_float(){
	return 2*((GLfloat)rand()/(GLfloat)RAND_MAX) - 1;
}

void display_heightmap(GLfloat **heightmap, int w, int h){
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			cout << heightmap[y][x] << "\t";
		}
		cout << endl;
	}
}

GLfloat **make_terramap(int power, GLfloat displace){
	int size = (int)pow((GLfloat)2,power);
	GLfloat **heightmap = create_heightmap(size, size);
	int level;
	for(level = 0; level < power; level++){
		int inc = size/(int)pow((GLfloat)2,level);
		int start = inc/2;
		int x;
		// squares to diamonds
		for(x = 0; x < size; x += inc){
			int y;
			for(y = 0; y < size; y += inc){
				heightmap[start+y][start+x] = (heightmap[y][x] + heightmap[y][(x+inc)%size] + heightmap[(y+inc)%size][x] + heightmap[(y+inc)%size][(x+inc)%size])/4.0 + rand_float()*(GLfloat)inc*displace;
			}
		}
		// diamonds to squares
		for(x = start; x < size; x += inc){
			int y;
			for(y = 0; y < size; y += inc){
				heightmap[y][x] = (heightmap[(y+inc/2)%size][x] + heightmap[y][(x+inc/2)%size] + heightmap[(size+y-inc/2)%size][x] + heightmap[y][(size+x-inc/2)%size])/4.0 + rand_float()*(GLfloat)inc*displace;
			}
		}
		for(x = 0; x < size; x += inc){
			int y;
			for(y = start; y < size; y += inc){
				heightmap[y][x] = (heightmap[(y+inc/2)%size][x] + heightmap[y][(x+inc/2)%size] + heightmap[(size+y-inc/2)%size][x] + heightmap[y][(size+x-inc/2)%size])/4.0 + rand_float()*(GLfloat)inc*displace;
			}
		}
	}
	return heightmap;
}

void draw_heightmap_vector(GLfloat **heightmap, int w, int h, GLfloat x_scale, GLfloat y_scale, GLfloat z_scale){
	int x;
	glColor3f(0,.1,0); // solid first
	glBegin(GL_QUADS);
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			glVertex3f((GLfloat)x*x_scale,heightmap[y%h][x%w]*z_scale,(GLfloat)y*y_scale);
			glVertex3f((GLfloat)x*x_scale,heightmap[(y+1)%h][x%w]*z_scale,(GLfloat)(y+1)*y_scale);
			glVertex3f((GLfloat)(x+1)*x_scale,heightmap[(y+1)%h][(x+1)%w]*z_scale,(GLfloat)(y+1)*y_scale);
			glVertex3f((GLfloat)(x+1)*x_scale,heightmap[y%h][(x+1)%w]*z_scale,(GLfloat)y*y_scale);
		}
	}
	glEnd();
	glTranslatef(0,.05,0); // up a bit so we don't get z fighting
	glColor3f(0,1,0); // now for vector lines
	glBegin(GL_LINES);
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			glVertex3f((GLfloat)x*x_scale,heightmap[y%h][x%w]*z_scale,(GLfloat)y*y_scale);
			glVertex3f((GLfloat)(x+1)*x_scale,heightmap[y%h][(x+1)%w]*z_scale,(GLfloat)y*y_scale);

			glVertex3f((GLfloat)x*x_scale,heightmap[y%h][x%w]*z_scale,(GLfloat)y*y_scale);
			glVertex3f((GLfloat)x*x_scale,heightmap[(y+1)%h][x%w]*z_scale,(GLfloat)(y+1)*y_scale);
		}
	}
	glEnd();
	glTranslatef(0,-.05,0);
}

void terrain_color(GLfloat **heightmap, int x, int y){
	if(heightmap[y][x] < 1){
		glColor3f(0,0,.3+heightmap[y][x]*.7);
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

void draw_heightmap_texture(GLfloat **heightmap, GLfloat ***normalmap, int w, int h, GLfloat x_scale, GLfloat y_scale, GLfloat z_scale){
	int x;
	glBegin(GL_QUADS);
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			terrain_color(heightmap,x%w,y%h);
			glTexCoord2d(((GLfloat)x)/(GLfloat)w,((GLfloat)y)/(GLfloat)h);
			glNormal3fv(normalmap[y][x]);
			glVertex3f((GLfloat)x*x_scale,heightmap[y%h][x%w]*z_scale,(GLfloat)y*y_scale);

			terrain_color(heightmap,x%w,(y+1)%h);
			glTexCoord2d(((GLfloat)x)/(GLfloat)w,((GLfloat)(y+1))/(GLfloat)h);
			glNormal3fv(normalmap[(y+1)%h][x]);
			glVertex3f((GLfloat)x*x_scale,heightmap[(y+1)%h][x%w]*z_scale,(GLfloat)(y+1)*y_scale);

			terrain_color(heightmap,(x+1)%w,(y+1)%h);
			glTexCoord2d(((GLfloat)(x+1))/(GLfloat)w,((GLfloat)(y+1))/(GLfloat)h);
			glNormal3fv(normalmap[(y+1)%h][(x+1)%w]);
			glVertex3f((GLfloat)(x+1)*x_scale,heightmap[(y+1)%h][(x+1)%w]*z_scale,(GLfloat)(y+1)*y_scale);

			terrain_color(heightmap,(x+1)%w,y%h);
			glTexCoord2d(((GLfloat)(x+1))/(GLfloat)w,((GLfloat)y)/(GLfloat)h);
			glNormal3fv(normalmap[y][(x+1)%w]);
			glVertex3f((GLfloat)(x+1)*x_scale,heightmap[y%h][(x+1)%w]*z_scale,(GLfloat)y*y_scale);
		}
	}
	glEnd();
}

GLfloat ***make_normalmap(GLfloat **heightmap, int w, int h){
	GLfloat ***normalmap = new GLfloat**[h];
	int y;
	for(y = 0; y < h; y++){
		normalmap[y] = new GLfloat*[w];
		int x;
		for(x = 0; x < w; x++){
			normalmap[y][x] = new GLfloat[3];
			GLfloat i,j,k;
			i = -(heightmap[y][(x+1)%w]-heightmap[y][x]);
			j = 1;
			k = -(heightmap[(y+1)%h][x]-heightmap[y][x]);
			GLfloat m = sqrt(i*i + j*j + k*k);
			normalmap[y][x][0] = i/m;
			normalmap[y][x][1] = j/m;
			normalmap[y][x][2] = k/m;
		}
	}
	return normalmap;
}

void oceanify(GLfloat **heightmap, int w, int h, GLfloat min){
	int x;
	bool doShift = false;
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			if(heightmap[y][x] < 0){
				doShift = true;
				heightmap[y][x] = (-1.0)/(heightmap[y][x]-1) - 1;
			}
		}
	}
	if(doShift){
		for(x = 0; x < w; x++){
			int y;
			for(y = 0; y < h; y++){
				heightmap[y][x] += 1;
			}
		}
	}
}

void hillify(GLfloat **heightmap, int w, int h, GLfloat flatness){
	oceanify(heightmap, w, h, 0.1); // no negatives, yo!
	int x;
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			heightmap[y][x] = (GLfloat)pow((GLfloat)heightmap[y][x],(GLfloat)flatness);
		}
	}
}

void smoothify(GLfloat **heightmap, int w, int h, GLfloat inertia){
	if(inertia > 1) inertia = 1;
	if(inertia < 0) inertia = 0;
	int x;
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			heightmap[y][x] = (heightmap[y][x]*inertia + (1-inertia)*(heightmap[(h+y-1)%h][x] + heightmap[y][(w+x-1)%w] + heightmap[(y+1)%h][x] + heightmap[y][(x+1)%w])/4.0);
		}
	}
}

void out_to_file(GLfloat **heightmap, int w, int h, string file){
	if(w > 0 && h > 0){
		ofstream of(file.c_str());
		of << "P2 " << w << " " << h << " 255\n";
		int y,x;
		GLfloat max = heightmap[0][0];
		GLfloat min = max;
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				if(heightmap[y][x] > max){
					max = heightmap[y][x];
				}
				if(heightmap[y][x] < min){
					min = heightmap[y][x];
				}
			}
		}
		for(y = 0; y < h; y++){
			for(x = 0; x < w; x++){
				of << (int)(255*(heightmap[y][x]-min)/(max-min)) << " ";
			}
			of << "\n";
		}
		of.close();
	}
}

