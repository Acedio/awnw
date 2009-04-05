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
#include "textures.h"

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

void draw_heightmap_vector(GLfloat **heightmap, int w, int h){
	int x;
	glColor3f(0,.1,0); // solid first
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2,2);
	glBegin(GL_QUADS);
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			glVertex3f((GLfloat)x,heightmap[y%h][x%w],(GLfloat)y);
			glVertex3f((GLfloat)x,heightmap[(y+1)%h][x%w],(GLfloat)(y+1));
			glVertex3f((GLfloat)(x+1),heightmap[(y+1)%h][(x+1)%w],(GLfloat)(y+1));
			glVertex3f((GLfloat)(x+1),heightmap[y%h][(x+1)%w],(GLfloat)y);
		}
	}
	glEnd();
	glColor3f(0,1,0); // now for vector lines
	glPolygonOffset(1,1);
	glBegin(GL_LINES);
	for(x = 0; x < w; x++){
		int y;
		for(y = 0; y < h; y++){
			glVertex3f((GLfloat)x,heightmap[y%h][x%w],(GLfloat)y);
			glVertex3f((GLfloat)(x+1),heightmap[y%h][(x+1)%w],(GLfloat)y);

			glVertex3f((GLfloat)x,heightmap[y%h][x%w],(GLfloat)y);
			glVertex3f((GLfloat)x,heightmap[(y+1)%h][x%w],(GLfloat)(y+1));
		}
	}
	glEnd();
	glDisable(GL_POLYGON_OFFSET_FILL);
}

inline void terrain_color(GLfloat **heightmap, int x, int y){
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

inline GLuint terrain_texture(GLfloat **heightmap, int x, int y, GLuint textures[TEXTURE_COUNT]){
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

void draw_heightmap_texture(GLfloat **heightmap, GLfloat ***normalmap, GLuint textures[TEXTURE_COUNT], int w, int h){
	glColor3f(1,1,1);
	//glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonOffset(2,2);
	//glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_SAND]);
	/*glBegin(GL_TRIANGLE_STRIP);
	for(int y = 0; y < h; y++){
		GLfloat fy = (GLfloat)(y)/32.0;
		GLfloat fy1 = (GLfloat)(1+y)/32.0;
		for(int x = 0; x <= w; x++){
			GLfloat fx = (GLfloat)(x)/32.0;

			glTexCoord2d(fx,fy);
			glNormal3fv(normalmap[y%h][x%w]);
			glVertex3f((GLfloat)x,((heightmap[y%h][x%w]<1)?1:heightmap[y%h][x%w]),(GLfloat)y);

			glTexCoord2d(fx,fy1);
			glNormal3fv(normalmap[(y+1)%h][x%w]);
			glVertex3f((GLfloat)x,((heightmap[(y+1)%h][x%w]<1)?1:heightmap[(y+1)%h][x%w]),(GLfloat)(y+1));
		}
		glTexCoord2d((GLfloat)(w%32)/32.0,fy1);
		glNormal3fv(normalmap[(y+1)%h][0]);
		glVertex3f((GLfloat)w,((heightmap[(y+1)%h][0]<1)?1:heightmap[(y+1)%h][0]),(GLfloat)(y+1));

		glTexCoord2d(0,fy1);
		glNormal3fv(normalmap[(y+1)%h][0]);
		glVertex3f((GLfloat)0,((heightmap[(y+1)%h][0]<1)?1:heightmap[(y+1)%h][0]),(GLfloat)(y+1));
	}
	glEnd();*/

	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CCW);

	//glDepthFunc(GL_LESS); // <-- Those stupid triangles are caused by something like this

	//glBlendFunc(GL_ONE, GL_ZERO);

	glActiveTexture(GL_TEXTURE0);
	//glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_ALPHA]);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_GRASS]);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE1);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

	glActiveTexture(GL_TEXTURE2);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE0);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE2);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_GRASS]);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(.5,.5,0);
	glRotatef(45,0,0,1);
	glTranslatef(-.5,-.5,0);
	glMatrixMode(GL_MODELVIEW);
	/*glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE2);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
	glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE2);
	glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);*/

	//glPolygonOffset(1,1);
	glBegin(GL_TRIANGLE_STRIP);
	for(int y = 0; y < h; y++){
		GLfloat fy = (GLfloat)(y)/32.0;
		GLfloat fy1 = (GLfloat)(1+y)/32.0;
		for(int x = 0; x <= w; x++){
			GLfloat fx = (GLfloat)(x)/32.0;

			glMultiTexCoord2f(GL_TEXTURE0,(GLfloat)x/(GLfloat)w,(GLfloat)y/(GLfloat)h);
			glMultiTexCoord2f(GL_TEXTURE1,(GLfloat)x/(GLfloat)w,(GLfloat)y/(GLfloat)h);
			glMultiTexCoord2f(GL_TEXTURE2,(GLfloat)x/(GLfloat)w,(GLfloat)y/(GLfloat)h);
			glNormal3fv(normalmap[y%h][x%w]);
			glVertex3f((GLfloat)x,((heightmap[y%h][x%w]<1)?1:heightmap[y%h][x%w]),(GLfloat)y);

			glMultiTexCoord2f(GL_TEXTURE0,(GLfloat)x/(GLfloat)w,(GLfloat)(y+1)/(GLfloat)h);
			glMultiTexCoord2f(GL_TEXTURE1,(GLfloat)x/(GLfloat)w,(GLfloat)(y+1)/(GLfloat)h);
			glMultiTexCoord2f(GL_TEXTURE2,(GLfloat)x/(GLfloat)w,(GLfloat)(y+1)/(GLfloat)h);
			glNormal3fv(normalmap[(y+1)%h][x%w]);
			glVertex3f((GLfloat)x,((heightmap[(y+1)%h][x%w]<1)?1:heightmap[(y+1)%h][x%w]),(GLfloat)(y+1));
		}
		glMultiTexCoord2f(GL_TEXTURE0,((GLfloat)1),(GLfloat)(y+1)/(GLfloat)h);
		glMultiTexCoord2f(GL_TEXTURE1,((GLfloat)1),(GLfloat)(y+1)/(GLfloat)h);
		glMultiTexCoord2f(GL_TEXTURE2,((GLfloat)1),(GLfloat)(y+1)/(GLfloat)h);
		glNormal3fv(normalmap[(y+1)%h][0]);
		glVertex3f((GLfloat)w,((heightmap[(y+1)%h][0]<1)?1:heightmap[(y+1)%h][0]),(GLfloat)(y+1));

		glMultiTexCoord2f(GL_TEXTURE0,0,(GLfloat)(y+1)/(GLfloat)h);
		glMultiTexCoord2f(GL_TEXTURE1,0,(GLfloat)(y+1)/(GLfloat)h);
		glMultiTexCoord2f(GL_TEXTURE2,0,(GLfloat)(y+1)/(GLfloat)h);
		glNormal3fv(normalmap[(y+1)%h][0]);
		glVertex3f((GLfloat)0,((heightmap[(y+1)%h][0]<1)?1:heightmap[(y+1)%h][0]),(GLfloat)(y+1));
	}
	glEnd();

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, NULL);
	glDisable(GL_TEXTURE_2D);

	//glDisable(GL_POLYGON_OFFSET_FILL);

	//glDepthFunc(GL_LESS);

	//glDisable(GL_BLEND);
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

GLfloat **normalize(GLfloat **heightmap, int w, int h, GLfloat depth){
	GLfloat min, max;
	min = max = heightmap[0][0];
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			if(heightmap[y][x] < min){
				min = heightmap[y][x];
			} else if(heightmap[y][x] > max){
				max = heightmap[y][x];
			}
		}
	}
	GLfloat **normalized = create_heightmap(w,h);
	for(int y = 0; y < h; y++){
		for(int x = 0; x < w; x++){
			normalized[y][x] = depth * (heightmap[y][x] - min) / (max - min);
		}
	}
	return normalized;
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
