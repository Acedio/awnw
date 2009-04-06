// RANDOM TERRAMAP
// By Josh Simmons 2009

//TODO: Fix normal map creation to set vertex normals equal to the mean of the six surrounding surface normals. http://www.gamedev.net/reference/programming/features/normalheightfield/

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
#include "textures.h"

const int SCREEN_W = 800;
const int SCREEN_H = 600;
const int SCREEN_BPP = 32;

const float PI = 3.1415;

bool running = true;

int power=7, size;
GLfloat hill_factor = .97;
bool spinning = false;
GLfloat spin = 0;
GLfloat spin_speed = 0.03;
bool textured = true;
bool day = true;
bool fog = true;
bool light = true;
bool flat = false;
GLfloat fog_amount = .006;
GLfloat **current_heightmap;
GLfloat ***current_normalmap;

GLfloat cloud_move_x = 0;
GLfloat cloud_move_y = 0;
GLfloat cloud_move_speed = .0005;
GLfloat cloud_move_angle = 1;

GLfloat x_off = 0, y_off = -50, z_off = -20, h_angle = 0, v_angle = 45;

GLuint textures[TEXTURE_COUNT];

GLuint cloud_texture;

GLuint terrain_dl;

void make_textures(){
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	if(glIsTexture(cloud_texture)){
		glDeleteTextures(1,&cloud_texture);
	}
	textures[TEXTURE_DEFAULT] = 0;
	if(glIsTexture(textures[TEXTURE_GRASS])){
		glDeleteTextures(1,&textures[TEXTURE_GRASS]);
	}
	if(glIsTexture(textures[TEXTURE_SAND])){
		glDeleteTextures(1,&textures[TEXTURE_SAND]);
	}
	if(glIsTexture(textures[TEXTURE_ALPHA])){
		glDeleteTextures(1,&textures[TEXTURE_ALPHA]);
	}
	cloud_texture = make_cloud_texture();
	textures[TEXTURE_SAND] = make_sand_texture();
	textures[TEXTURE_GRASS] = make_grass_texture();

	GLfloat *perlin = perlin_noise(power,power,.5,1,power-1);
	cloudify(perlin, size, size, .25, .01);
	/*GLfloat *perlin = new GLfloat[size*size];
	for(int i = 0; i < size*size; i++){
		perlin[i] = (GLfloat)i/(GLfloat)(size*size);
	}*/
	GLuint texture;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	glGenTextures(1,&texture);
	glBindTexture(GL_TEXTURE_2D,texture);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D,GL_ALPHA,size,size,GL_ALPHA,GL_FLOAT,perlin);

	delete[] perlin;

	textures[TEXTURE_ALPHA] = texture;

	glEnable(GL_TEXTURE_2D);
}

void draw_terrain(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Transforms for camera view
	glLoadIdentity();
	glRotatef(h_angle,0,1,0);
	glRotatef(v_angle,cos(PI*h_angle/180.0),0,sin(PI*h_angle/180.0));
	glTranslatef(x_off-size/2,y_off,z_off-size);
	glTranslatef(size/2,0,size/2); // Translate so we're rotating around the center of the land
	glRotatef(spin,0,1,0); // Yoshi's Island spin!
	glTranslatef(-size/2,0,-size/2); // Translate back
	if(spinning){
		spin += spin_speed;
	}
	if(textured){
		glCallList(terrain_dl);
	} else {
		draw_heightmap_vector(current_heightmap,size,size);
	}
	glTranslatef(size/2,0,size/2); // Translate to center so the clouds are drawn around the center
	glRotatef(-spin,0,1,0); // Spin back so the clouds don't rotate with the land

	// draw cloud spehere

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, cloud_texture);
	glColor3f(1,1,1);
	if(light){
		glDisable(GL_LIGHTING); // We want our clouds to be bright and shiny and unaffected by lighting
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_LIGHT0);
	}
	glBegin(GL_QUADS);
	double radius = 6*size;
	double max = .9*sqrt(radius*radius/2);
	int divisions = 6; // number of x and z divisions in the cloud layer
	double step = 2*max/(double)divisions;
	double lower_by = 5*radius/6;
	for(int z = -divisions/2; z < divisions/2; z += 1){
		for(int x = -divisions/2; x < divisions/2; x += 1){
			glTexCoord2d(0+cloud_move_x,0+cloud_move_y); glVertex3f(x*step,sqrt(radius*radius - x*x*step*step - z*z*step*step)-lower_by,z*step);
			glTexCoord2d(1+cloud_move_x,0+cloud_move_y); glVertex3f((x+1)*step,sqrt(radius*radius - ((x+1)*step)*((x+1)*step) - z*z*step*step)-lower_by,z*step);
			glTexCoord2d(1+cloud_move_x,1+cloud_move_y); glVertex3f((x+1)*step,sqrt(radius*radius - ((x+1)*step)*((x+1)*step) - ((z+1)*step)*((z+1)*step))-lower_by,(z+1)*step);
			glTexCoord2d(0+cloud_move_x,1+cloud_move_y); glVertex3f(x*step,sqrt(radius*radius - x*x*step*step - ((z+1)*step)*((z+1)*step))-lower_by,(z+1)*step);
		}
	}
	glEnd();

	// Nice slow cloud movement and checks to see if we can set s and t back to 0 for tex repeating
	cloud_move_x += cos(cloud_move_angle)*cloud_move_speed;
	while(cloud_move_x > 1.0){
		cloud_move_x -= 1.0;
	}

	cloud_move_y += sin(cloud_move_angle)*cloud_move_speed;
	while(cloud_move_y > 1.0){
		cloud_move_y -= 1.0;
	}

	if(light){
		// Bring the light back for terrain drawing. We want lighting enabled during
		// the keyboard routine so we can still change light properties (position, etc)
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
		glEnable(GL_LIGHT0);
	}
}

void resize(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f,1.0f*w/h,1.0f,650.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(Uint8 *keys){
	GLfloat speed = .5;
	if(keys[SDLK_w]){
		x_off -= cos(PI*v_angle/180.0)*sin(PI*h_angle/180.0)*speed;
		y_off += sin(PI*v_angle/180.0)*speed;
		z_off += cos(PI*v_angle/180.0)*cos(PI*h_angle/180.0)*speed;
	}
	if(keys[SDLK_s]){
		x_off += cos(PI*v_angle/180.0)*sin(PI*h_angle/180.0)*speed;
		y_off -= sin(PI*v_angle/180.0)*speed;
		z_off -= cos(PI*v_angle/180.0)*cos(PI*h_angle/180.0)*speed;
	}
	if(keys[SDLK_a]){
		x_off += cos(PI*h_angle/180.0)*speed;
		z_off += sin(PI*h_angle/180.0)*speed;
	}
	if(keys[SDLK_d]){
		x_off -= cos(PI*h_angle/180.0)*speed;
		z_off -= sin(PI*h_angle/180.0)*speed;
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
		current_heightmap = normalize(current_heightmap, size, size, 15);
		current_normalmap = make_normalmap(current_heightmap,size,size);
		make_textures();
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
		running = false;
	}
	if(keys[SDLK_p]){
		out_to_file(current_heightmap, size, size, "output.pgm");
	}
	if(keys[SDLK_t]){
		textured = !textured;
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
			glFogf(GL_FOG_DENSITY, fog_amount);
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
			size = (int)pow((GLfloat)2,power);
			current_heightmap = make_terramap(power,.25);
			//current_heightmap = normalize(current_heightmap, size, size, 15);
			if(textured){
				//oceanify(current_heightmap, size, size, 0.1);
			}
			current_normalmap = make_normalmap(current_heightmap,size,size);
		}
	}
}

GLfloat **d1_to_d2(GLfloat *one, int w, int h){
	GLfloat **two = new GLfloat*[h];
	for(int y = 0; y < h; y++){
		two[y] = new GLfloat[w];
		for(int x = 0; x < w; x++){
			two[y][x] = one[y*w+x];
		}
	}
	return two;
}

int main(int argc, char **argv){
	srand(time(0));
	size = (int)pow((GLfloat)2,power);
	//current_heightmap = make_terramap(power,.25);
	GLfloat *perlin = perlin_noise(power,power,.5,1,power-1);
	current_heightmap = d1_to_d2(perlin, size, size);
	delete[] perlin;
	current_heightmap = normalize(current_heightmap, size, size, 15);
	//oceanify(current_heightmap, size, size, 0.1);
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

	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
	glEnable(GL_LIGHT0);

	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_EXP2);
	GLfloat fog_color[] = {0.8, 0.8, 0.8, 1.0};
	glFogfv(GL_FOG_COLOR, fog_color);
	glFogf(GL_FOG_DENSITY, fog_amount);
	glHint(GL_FOG_HINT, GL_NICEST);

	glClearColor(0,0,0,1);

	resize(cur_screen_w, cur_screen_h);

	GLfloat light_pos[] = {0,50,0,1.0};

	make_textures();

	terrain_dl = glGenLists(1);
	
	glNewList(terrain_dl,GL_COMPILE);
	draw_heightmap_texture(current_heightmap,current_normalmap,textures,size,size);
	glEndList();

	SDL_WM_GrabInput(SDL_GRAB_ON);

	SDL_ShowCursor(SDL_DISABLE);

	int frames = 0;
	int start = SDL_GetTicks();

	GLint max;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max);
	cout << "max: " << max << endl;

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
		if(pressed&SDL_BUTTON(2)){
			SDL_ShowCursor(SDL_ENABLE);
			light_pos[0] = size * (GLfloat)x / (GLfloat)cur_screen_w - size / 2;
			light_pos[2] = size * (GLfloat)y / (GLfloat)cur_screen_h - size / 2;
		} else {
			SDL_ShowCursor(SDL_DISABLE);
		}

		if(pressed&SDL_BUTTON(1)){
			light_pos[1] += 0.5;
		}
		if(pressed&SDL_BUTTON(3)){
			light_pos[1] -= 0.5;
		}

		glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

		pressed = SDL_GetRelativeMouseState(&x, &y);

		if(!(pressed&SDL_BUTTON(2))){
			h_angle += x;
			v_angle += y;
			if(v_angle < -90) v_angle = -90;
			if(v_angle > 90) v_angle = 90;
		}

		keyboard(keys);

		draw_terrain();

		SDL_GL_SwapBuffers();
		frames++;
		if(frames >= 1000){
			int millis = SDL_GetTicks() - start;
			cout << (1000 * frames) / millis << " FPS" << endl;
			frames = 0;
			start = SDL_GetTicks();
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1,&cloud_texture);
	glDeleteTextures(1,&textures[TEXTURE_GRASS]);
	glDeleteTextures(1,&textures[TEXTURE_SAND]);

	glDeleteLists(terrain_dl,1);

	SDL_WM_GrabInput(SDL_GRAB_OFF);

	SDL_FreeSurface(screen);

	SDL_Quit();
	for(int i = 0; i < size; i++){
		delete[] current_heightmap[i];
	}
	delete[] current_heightmap;

	for(int i = 0; i < size; i++){
		for(int j = 0; j < size; j++){
			delete[] current_normalmap[i][j];
		}
		delete[] current_normalmap[i];
	}
	delete[] current_normalmap;
	return 0;
}
