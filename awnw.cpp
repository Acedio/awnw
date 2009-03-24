// RANDOM TERRAMAP
// By Josh Simmons 2009

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>

using namespace std;

#include "terramap.h"
#include "perlin.h"

#define SCREEN_W 500
#define SCREEN_H 500
#define SCREEN_BPP 32

int power=6, size;
GLfloat hill_factor = .97;
bool spinning = false;
GLfloat spin = 0;
GLfloat spin_speed = 0.03;
bool textured = true;
bool day = true;
bool fog = false;
bool light = false;
bool flat = false;
GLfloat **current_heightmap;
GLfloat ***current_normalmap;

GLfloat x_off = 0, y_off = -50, z_off = -20, h_angle = 0, v_angle = 45;

GLuint cloud_texture;
GLuint ground_texture;

GLfloat sky_pos = 60;

void draw_terrain(){
	glLoadIdentity();
	glRotatef(h_angle,0,1,0);
	glRotatef(v_angle,cos(3.1415*h_angle/180.0),0,sin(3.1415*h_angle/180.0));
	glTranslatef(x_off-size/2,y_off,z_off-size);
	glTranslatef(size/2,0,size/2);
	glRotatef(spin,0,1,0);
	glTranslatef(-size/2,0,-size/2);
	if(spinning){
		spin += spin_speed;
	}
	//glBindTexture(GL_TEXTURE_2D, ground_texture);
	glDisable(GL_TEXTURE_2D);
	if(textured){
		draw_heightmap_texture(current_heightmap,current_normalmap,size,size,1,1,1);
	} else {
		draw_heightmap_vector(current_heightmap,size,size,1,1,1);
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, cloud_texture);
	glColor3f(1,1,1);
	glBegin(GL_QUADS);
	glTexCoord2d(0,1); glVertex3f(-1*pow(2,power),sky_pos,-1*pow(2,power));
	glTexCoord2d(1,1); glVertex3f(2*pow(2,power),sky_pos,-1*pow(2,power));
	glTexCoord2d(1,0); glVertex3f(2*pow(2,power),sky_pos,2*pow(2,power));
	glTexCoord2d(0,0); glVertex3f(-1*pow(2,power),sky_pos,2*pow(2,power));

	glTexCoord2d(0,0); glVertex3f(-1*pow(2,power),sky_pos,-1*pow(2,power));
	glTexCoord2d(1,0); glVertex3f(2*pow(2,power),sky_pos,-1*pow(2,power));
	glTexCoord2d(1,1); glVertex3f(2*pow(2,power),sky_pos-3*pow(2,power),-1*pow(2,power));
	glTexCoord2d(0,1); glVertex3f(-1*pow(2,power),sky_pos-3*pow(2,power),-1*pow(2,power));

	glTexCoord2d(0,1); glVertex3f(2*pow(2,power),sky_pos-3*pow(2,power),-1*pow(2,power));
	glTexCoord2d(1,1); glVertex3f(2*pow(2,power),sky_pos-3*pow(2,power),2*pow(2,power));
	glTexCoord2d(1,0); glVertex3f(2*pow(2,power),sky_pos,2*pow(2,power));
	glTexCoord2d(0,0); glVertex3f(2*pow(2,power),sky_pos,-1*pow(2,power));
	glEnd();
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f,1.0f*w/h,1.0f,350.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(Uint8 *keys){
	GLfloat speed = .5;
	if(keys[SDLK_w]){
		x_off -= sin(3.1415*h_angle/180.0)*speed;
		z_off += cos(3.1415*h_angle/180.0)*speed;
	}
	if(keys[SDLK_s]){
		x_off += sin(3.1415*h_angle/180.0)*speed;
		z_off -= cos(3.1415*h_angle/180.0)*speed;
	}
	if(keys[SDLK_a]){
		x_off += cos(3.1415*h_angle/180.0)*speed;
		z_off += sin(3.1415*h_angle/180.0)*speed;
	}
	if(keys[SDLK_d]){
		x_off -= cos(3.1415*h_angle/180.0)*speed;
		z_off -= sin(3.1415*h_angle/180.0)*speed;
	}
	if(keys[SDLK_r]){
		y_off -= speed;
	}
	if(keys[SDLK_f]){
		y_off += speed;
	}
	if(keys[SDLK_j]){
		h_angle -= 3*speed;
	}
	if(keys[SDLK_l]){
		h_angle += 3*speed;
	}
	if(keys[SDLK_i]){
		v_angle -= 3*speed;
	}
	if(keys[SDLK_k]){
		v_angle += 3*speed;
	}
	if(keys[SDLK_z]){
		current_heightmap = make_terramap(power,.25);
		if(textured){
			oceanify(current_heightmap, size, size, 0.1);
		}
		current_normalmap = make_normalmap(current_heightmap,size,size);
		glDeleteTextures(1,&cloud_texture);
		cloud_texture = make_cloud_texture();
	}
	if(keys[SDLK_o]){
		oceanify(current_heightmap, size, size, 0.1);
		current_normalmap = make_normalmap(current_heightmap,size,size);
	}
	if(keys[SDLK_h]){
		hillify(current_heightmap, size, size, hill_factor);
		current_normalmap = make_normalmap(current_heightmap,size,size);
	}
	if(keys[SDLK_y]){
		hillify(current_heightmap, size, size, 1.0/hill_factor);
		current_normalmap = make_normalmap(current_heightmap,size,size);
	}
	if(keys[SDLK_g]){
		smoothify(current_heightmap, size, size, .9);
		current_normalmap = make_normalmap(current_heightmap,size,size);
	}
	if(keys[SDLK_u]){
		spinning = !spinning;
	}
	if(keys[SDLK_KP_MINUS]){
		spin_speed -= .01;
	}
	if(keys[SDLK_KP_PLUS]){
		spin_speed += .01;
	}
	if(keys[SDLK_q]){
		exit(0);
	}
	if(keys[SDLK_p]){
		out_to_file(current_heightmap, size, size, "output.pgm");
	}
	if(keys[SDLK_t]){
		textured = !textured;
		if(textured){
			oceanify(current_heightmap, size, size, 0.1);
		}
		current_normalmap = make_normalmap(current_heightmap,size,size);
	}
	if(keys[SDLK_c]){
		light = !light;
		if(light){
			glEnable(GL_LIGHTING);
			glEnable(GL_COLOR_MATERIAL);
			glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
			glEnable(GL_LIGHT0);
		} else {
			glDisable(GL_LIGHTING);
			glDisable(GL_COLOR_MATERIAL);
			glDisable(GL_LIGHT0);
		}
	}
	if(keys[SDLK_e]){
		day = !day;
		if(day){
			GLfloat ambient[] = {0.0, 0.0, 0.0, 1.0};
			GLfloat diffuse[] = {1.0, 1.0, 1.0, 1.0};
			glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		} else {
			GLfloat ambient[] = {0.0, 0.0, 0.0, 0.0};
			GLfloat diffuse[] = {0.2, 0.2, 0.6, 1.0};
			glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
			glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		}
	}
	if(keys[SDLK_x]){
		fog = !fog;
		if(fog){
			glEnable(GL_FOG);
			glFogi(GL_FOG_MODE, GL_EXP2);
			GLfloat fog_color[] = {0.8, 0.8, 0.8, 1.0};
			glFogfv(GL_FOG_COLOR, fog_color);
			glFogf(GL_FOG_DENSITY, 0.01);
			glHint(GL_FOG_HINT, GL_NICEST);
		} else {
			glDisable(GL_FOG);
		}
	}
	if(keys[SDLK_m]){
		flat = !flat;
		if(flat){
			glShadeModel(GL_FLAT);
		} else {
			glShadeModel(GL_SMOOTH);
		}
	}
	Uint8 key;
	for(key = SDLK_1; key <= SDLK_9; key++){
		if(keys[key]){
			power = key - SDLK_0;
			size = pow(2,power);
			current_heightmap = make_terramap(power,.25);
			if(textured){
				oceanify(current_heightmap, size, size, 0.1);
			}
			current_normalmap = make_normalmap(current_heightmap,size,size);
		}
	}
}

int main(int argc, char **argv){
	srand(time(0));
	size = pow(2,power);
	current_heightmap = make_terramap(power,.25);
	oceanify(current_heightmap, size, size, 0.1);
	current_normalmap = make_normalmap(current_heightmap,size,size);

	SDL_Surface *screen;

	SDL_Init(SDL_INIT_VIDEO);

	int cur_screen_w = SCREEN_W, cur_screen_h = SCREEN_H;

	screen = SDL_SetVideoMode(cur_screen_w, cur_screen_h, SCREEN_BPP, SDL_OPENGL|SDL_RESIZABLE);

	if(screen == NULL){
		cout << "Could not initialize screen." << endl;
		exit(1);
	}

	SDL_WM_SetCaption("Terrain!", NULL);

	glEnable(GL_DEPTH_TEST); // for z buffering
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);

	glClearColor(0,0,0,1);

	resize(cur_screen_w, cur_screen_h);

	bool running = true;

	GLfloat light_pos[] = {0,15,0,1.0};

	cloud_texture = make_cloud_texture();

	ground_texture = make_ground_texture();

	while(running){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				running = false;
			} else if(event.type == SDL_VIDEORESIZE){
				SDL_FreeSurface(screen);
				screen = SDL_SetVideoMode(event.resize.w, event.resize.h, SCREEN_BPP, SDL_OPENGL|SDL_RESIZABLE);
				resize(event.resize.w,event.resize.h);
				cur_screen_w = event.resize.w;
				cur_screen_h = event.resize.h;
				if(screen == NULL){
					cout << "Could not resize screen." << endl;
					exit(1);
				}
			}
		}
		Uint8 *keys = SDL_GetKeyState(NULL);
		if(keys[SDLK_ESCAPE]){
			running = false;
		}

		int x, y;

		Uint8 pressed = SDL_GetMouseState(&x, &y);
		light_pos[0] = pow(2,power) * (GLfloat)x / (GLfloat)cur_screen_w;

		if(pressed&SDL_BUTTON(1)){
			light_pos[1] += 0.5;
			sky_pos += 1;
		}
		if(pressed&SDL_BUTTON(3)){
			light_pos[1] -= 0.5;
			sky_pos -= 1;
		}

		light_pos[2] = pow(2,power) * (GLfloat)y / (GLfloat)cur_screen_h;

		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

		keyboard(keys);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		draw_terrain();

		SDL_GL_SwapBuffers();
	}

	SDL_FreeSurface(screen);

	SDL_Quit();

	delete[] current_heightmap;
	return 0;
}
