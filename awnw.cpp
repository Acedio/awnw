// A WHOLE NEW WORLD
// By Josh Simmons 2009

//TODO: Fix normal map creation to set vertex normals equal to the mean of the six surrounding surface normals. http://www.gamedev.net/reference/programming/features/normalheightfield/
//TODO: Add more procedural textures (water)

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
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

const GLfloat PI = 3.1415;

bool running = true;

int power=7, size;
GLfloat max_terrain_height;
const GLfloat max_height_div = 6.0;
int max_textures;
GLfloat hill_factor = .97;
bool spinning = false;
GLfloat spin = 0;
GLfloat spin_speed = 0.03;
bool textured = true;
bool day = true;
bool fog = true;
bool light = true;
GLfloat light_pos[4] = {0,1,0,1.0};
bool flat = false;
bool flying = false;
GLfloat speed = .2;
GLfloat fog_amount = .005;
GLfloat fog_decay = .47;

GLfloat *current_heightmap = NULL;
GLfloat **current_normalmap = NULL;

GLfloat cloud_move_x = 0;
GLfloat cloud_move_y = 0;
GLfloat cloud_move_speed = .0005;
GLfloat cloud_move_angle = 1;

GLfloat water_t = 0;

GLfloat x_off = 0, y_off = 0, z_off = 0, h_angle = -45, v_angle = 0;

GLuint terrain_textures[TEXTURE_COUNT];

GLuint cloud_texture;

GLuint tree_texture;
GLfloat *treemap = NULL;
GLuint treemap_dl;
int tree_seed;

const int player_height = 3;

GLuint terrain_dl;

void make_textures(){
// creates and recreates all textures and texture alpha fades
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	if(glIsTexture(cloud_texture)){
		glDeleteTextures(1,&cloud_texture);
	}
	if(glIsTexture(tree_texture)){
		glDeleteTextures(1,&tree_texture);
	}
	terrain_textures[TEXTURE_NONE] = 0;
	if(glIsTexture(terrain_textures[TEXTURE_SAND])){
		glDeleteTextures(1,&terrain_textures[TEXTURE_SAND]);
	}
	if(glIsTexture(terrain_textures[TEXTURE_GRASS])){
		glDeleteTextures(1,&terrain_textures[TEXTURE_GRASS]);
	}
	if(glIsTexture(terrain_textures[TEXTURE_GRASS_ALPHA])){
		glDeleteTextures(1,&terrain_textures[TEXTURE_GRASS_ALPHA]);
	}
	if(glIsTexture(terrain_textures[TEXTURE_ROCK])){
		glDeleteTextures(1,&terrain_textures[TEXTURE_ROCK]);
	}
	if(glIsTexture(terrain_textures[TEXTURE_ROCK_ALPHA])){
		glDeleteTextures(1,&terrain_textures[TEXTURE_ROCK_ALPHA]);
	}
	cloud_texture = make_cloud_texture();
	tree_texture = make_tree_texture();
	terrain_textures[TEXTURE_SAND] = make_sand_texture();
	terrain_textures[TEXTURE_GRASS] = make_grass_texture();
	terrain_textures[TEXTURE_ROCK] = make_rock_texture();

	terrain_textures[TEXTURE_GRASS_ALPHA] = range_fade_texture(current_heightmap, size, size, .2*max_terrain_height, .5*max_terrain_height, 1*max_terrain_height, 1.1*max_terrain_height);
	terrain_textures[TEXTURE_ROCK_ALPHA] = range_fade_texture(current_heightmap, size, size, .8*max_terrain_height, .9*max_terrain_height, 1*max_terrain_height, 1.1*max_terrain_height);
}

GLfloat ground_height(GLfloat x, GLfloat z){
// returns the (interpolated) height at the given (x,z) coord
	GLfloat dx = x - (GLfloat)((int)x); // this is a stupid way to do this.
	GLfloat dz = z - (GLfloat)((int)z);
	GLfloat y00 = current_heightmap[(int)floor(size-1-z)*size+(int)floor(size-1-x)];
	GLfloat y01 = current_heightmap[(int)floor(size-1-z)*size+(int)floor(size-1-x-1)];
	GLfloat y0mid = y00+smoothstep(dx)*(y01-y00);
	GLfloat y10 = current_heightmap[(int)floor(size-1-z-1)*size+(int)floor(size-1-x)];
	GLfloat y11 = current_heightmap[(int)floor(size-1-z-1)*size+(int)floor(size-1-x-1)];
	GLfloat y1mid = y10+smoothstep(dx)*(y11-y10);
	GLfloat height = y0mid+smoothstep(dz)*(y1mid-y0mid);
	return height;
}

void draw_forests(){
// draws the current treemap
	glColor3f(1,1,1);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, tree_texture);

	glAlphaFunc(GL_GREATER, 0.7);
	glEnable(GL_ALPHA_TEST);

	glNormal3f(0,1,0);

	glBegin(GL_QUADS);
	for(int z = 0; z < size; z++){
		for(int x = 0; x < size; x++){
			if(treemap[z*size+x] > .9){
				glTexCoord2f(0,1);
				glVertex3f(x-.75+2*(treemap[z*size+x]-.75),current_heightmap[z*size+x],z+2*(treemap[z*size+x]-.75));
				glTexCoord2f(0,0);
				glVertex3f(x-.75+2*(treemap[z*size+x]-.75),4*(treemap[z*size+x]-.75)*2+current_heightmap[z*size+x],z+2*(treemap[z*size+x]-.75));
				glTexCoord2f(1,0);
				glVertex3f(x+.75+2*(treemap[z*size+x]-.75),4*(treemap[z*size+x]-.75)*2+current_heightmap[z*size+x],z+2*(treemap[z*size+x]-.75));
				glTexCoord2f(1,1);
				glVertex3f(x+.75+2*(treemap[z*size+x]-.75),current_heightmap[z*size+x],z+2*(treemap[z*size+x]-.75));

				glTexCoord2f(0,1);
				glVertex3f(x+2*(treemap[z*size+x]-.75),current_heightmap[z*size+x],z-.75+2*(treemap[z*size+x]-.75));
				glTexCoord2f(0,0);
				glVertex3f(x+2*(treemap[z*size+x]-.75),4*(treemap[z*size+x]-.75)*2+current_heightmap[z*size+x],z-.75+2*(treemap[z*size+x]-.75));
				glTexCoord2f(1,0);
				glVertex3f(x+2*(treemap[z*size+x]-.75),4*(treemap[z*size+x]-.75)*2+current_heightmap[z*size+x],z+.75+2*(treemap[z*size+x]-.75));
				glTexCoord2f(1,1);
				glVertex3f(x+2*(treemap[z*size+x]-.75),current_heightmap[z*size+x],z+.75+2*(treemap[z*size+x]-.75));
			}
		}
	}
	glEnd();

	glDisable(GL_ALPHA_TEST);

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_BLEND);
}

void make_treemap(int seed){
// creates (or recreates) the treemap for the current terrain
	if(treemap != NULL){
		delete[] treemap;
	}
	treemap = range_fade(current_heightmap, size, size, .4*max_terrain_height, .5*max_terrain_height, .8*max_terrain_height, .9*max_terrain_height);
	for(int z = 0; z < size; z++){
		treemap[z*size] = 0; // we don't want trees on the edge
		for(int x = 1; x < size; x++){
			if(z == 0){
				treemap[z*size+x] = 0; // no edge trees
			} else {
				treemap[z*size+x] = noise(seed+z*size+x)*treemap[z*size+x];
			}
		}
	}
	glDeleteLists(treemap_dl,1);
	treemap_dl = glGenLists(1);
	
	glNewList(treemap_dl,GL_COMPILE);
	draw_forests();
	glEndList();
}

void make_terrain(){
// (re)creates the display list for the terrain
	glDeleteLists(terrain_dl,1);
	terrain_dl = glGenLists(1);
	
	glNewList(terrain_dl,GL_COMPILE);
	draw_heightmap_texture(current_heightmap,current_normalmap,terrain_textures,max_textures,size,size);
	glEndList();
}

void draw_clouds(){
// draws the moving clouds. preeeeettttty :D
	glTranslatef(size/2,0,size/2); // Translate to center so the clouds are drawn around the center
	// draw cloud spehere
	
	glColor3f(1,1,1);

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

	// Draw the rest of the skybox
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		//back
		glVertex3f(-max,radius-lower_by,-max);
		glVertex3f(-max,-lower_by,-max);
		glVertex3f(max,-lower_by,-max);
		glVertex3f(max,radius-lower_by,-max);
		
		//left
		glVertex3f(-max,radius-lower_by,max);
		glVertex3f(-max,-lower_by,max);
		glVertex3f(-max,-lower_by,-max);
		glVertex3f(-max,radius-lower_by,-max);
		
		//front
		glVertex3f(max,radius-lower_by,max);
		glVertex3f(max,-lower_by,max);
		glVertex3f(-max,-lower_by,max);
		glVertex3f(-max,radius-lower_by,max);

		//right
		glVertex3f(max,radius-lower_by,-max);
		glVertex3f(max,-lower_by,-max);
		glVertex3f(max,-lower_by,max);
		glVertex3f(max,radius-lower_by,max);
	glEnd();
	glEnable(GL_TEXTURE_2D);

	if(light){
		// Bring the light back for terrain drawing. We want lighting enabled during
		// the keyboard routine so we can still change light properties (position, etc)
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
		glEnable(GL_LIGHT0);
	}
	glTranslatef(-size/2,0,-size/2);
}

void draw_water(GLfloat water_level, GLfloat ripple, GLfloat t){
// draws our nice fake wavy water
	glDisable(GL_TEXTURE_2D);

	glEnable(GL_LIGHTING);
	//glEnable(GL_COLOR_MATERIAL);
	//glColorMaterial( GL_FRONT_AND_BACK, GL_SPECULAR ) ;
	//glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE) ;
	//glEnable(GL_LIGHT0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// specular didn't really look that great...
	//GLfloat specular[] = {.5,.5,.5,.5};
	//glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	int water_res = 4;
	glBegin(GL_QUADS);

	glColor4f(0,.6,.8,.90);
	glNormal3f(0,1,0); // we don't need no shtinking normals!
	int z_start, z_end, z_dir, x_start, x_end, x_dir;
	if(cos(PI*h_angle/180.0) < 0){
		z_start = 0;
		z_end = size;
		z_dir = 1;
	} else {
		z_start = size-water_res;
		z_end = -water_res;
		z_dir = -1;
	}
	if(sin(PI*h_angle/180.0) > 0){
		x_start = 0;
		x_end = size;
		x_dir = 1;
	} else {
		x_start = size-water_res;
		x_end = -water_res;
		x_dir = -1;
	}
	for(int z = z_start; z_dir*z < z_dir*z_end; z += z_dir*water_res){
		for(int x = x_start; x_dir*x < x_dir*x_end; x += x_dir*water_res){
			GLfloat dx = -ripple*sin((GLfloat)x+3*t);
			GLfloat dz = ripple*cos((GLfloat)z+2*t);
			GLfloat l = sqrt(dx*dx+dz*dz+1);
			glNormal3f(dx/l, 1.0/l, dz/l);
			glVertex3f(x,ripple*cos((GLfloat)x+t*3)+ripple*sin((GLfloat)z+t*2)+water_level,z);

			dz = ripple*cos((GLfloat)(z+water_res)+2*t);
			l = sqrt(dx*dx+dz*dz+1);
			glNormal3f(dx/l, 1.0/l, dz/l);
			glVertex3f(x,ripple*cos((GLfloat)x+t*3)+ripple*sin((GLfloat)(z+water_res)+t*2)+water_level,z+water_res);

			dx = -ripple*sin((GLfloat)(x+water_res)+3*t);
			l = sqrt(dx*dx+dz*dz+1);
			glNormal3f(dx/l, 1.0/l, dz/l);
			glVertex3f(x+water_res,ripple*cos((GLfloat)(x+water_res)+t*3)+ripple*sin((GLfloat)(z+water_res)+t*2)+water_level,z+water_res);

			dz = ripple*cos((GLfloat)z+2*t);
			l = sqrt(dx*dx+dz*dz+1);
			glNormal3f(dx/l, 1.0/l, dz/l);
			glVertex3f(x+water_res,ripple*cos((GLfloat)(x+water_res)+t*3)+ripple*sin((GLfloat)z+t*2)+water_level,z);
		}
	}
	glEnd();
	glDisable(GL_BLEND);

	//specular[0] = specular[1] = specular[2] = specular[3] = 0;
	//glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glDisable(GL_LIGHTING);

	glEnable(GL_TEXTURE_2D);
}

void draw_scene(){
// draws the whole world
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	// Transforms for camera view
	glRotatef(h_angle,0,1,0);
	glRotatef(v_angle,cos(PI*h_angle/180.0),0,sin(PI*h_angle/180.0));
	glTranslatef(x_off-size,-y_off,z_off-size);
	glTranslatef(size/2,0,size/2);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glTranslatef(-size/2,0,-size/2);

	// Draw our stuff!
	draw_clouds();

	glTranslatef(size/2,0,size/2); // Translate so we're rotating around the center of the land
	glRotatef(spin,0,1,0); // Yoshi's Island spin!
	glTranslatef(-size/2,0,-size/2); // Translate back

	if(textured){
		glCallList(terrain_dl);
	} else {
		draw_heightmap_vector(current_heightmap,size,size);
	}
	glCallList(treemap_dl);
	draw_water(.35*max_terrain_height,.2,water_t);

	glTranslatef(0,0,-size);
	glTranslatef(size/2,0,size/2); // Translate so we're rotating around the center of the land
	glRotatef(-spin,0,1,0); // Yoshi's Island spin!
	glTranslatef(-size/2,0,-size/2); // Translate back

	if(spinning){
		spin += spin_speed;
	}

	water_t += .005;
}

void resize(int w, int h)
{
// simple reshape function. set's up a perspective viewing volume.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(45.0f,1.0f*w/h,1.0f,2000.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void keyboard(Uint8 *keys){
// big ol' key function that acts on user input
	if(keys[SDLK_w]){
		if(flying){
			x_off -= cos(PI*v_angle/180.0)*sin(PI*h_angle/180.0)*speed;
			z_off += cos(PI*v_angle/180.0)*cos(PI*h_angle/180.0)*speed;
			y_off -= sin(PI*v_angle/180.0)*speed;
		} else {
			x_off -= sin(PI*h_angle/180.0)*speed;
			z_off += cos(PI*h_angle/180.0)*speed;
			if(x_off < 0){
				x_off = 0;
			} else if(x_off > size - 1){
				x_off = size - 1;
			}
			if(z_off < 0){
				z_off = 0;
			} else if(z_off > size - 1){
				z_off = size - 1;
			}
			y_off = ground_height(x_off, z_off)+player_height;
		}
	}
	if(keys[SDLK_s]){
		if(flying){
			x_off += cos(PI*v_angle/180.0)*sin(PI*h_angle/180.0)*speed;
			z_off -= cos(PI*v_angle/180.0)*cos(PI*h_angle/180.0)*speed;
			y_off += sin(PI*v_angle/180.0)*speed;
		} else {
			x_off += sin(PI*h_angle/180.0)*speed;
			z_off -= cos(PI*h_angle/180.0)*speed;
			if(x_off < 0){
				x_off = 0;
			} else if(x_off > size - 1){
				x_off = size - 1;
			}
			if(z_off < 0){
				z_off = 0;
			} else if(z_off > size - 1){
				z_off = size - 1;
			}
			y_off = ground_height(x_off, z_off)+player_height;
		}
	}
	if(keys[SDLK_a]){
		if(flying){
			x_off += cos(PI*h_angle/180.0)*speed;
			z_off += sin(PI*h_angle/180.0)*speed;
		} else {
			x_off += cos(PI*h_angle/180.0)*speed;
			z_off += sin(PI*h_angle/180.0)*speed;
			if(x_off < 0){
				x_off = 0;
			} else if(x_off > size - 1){
				x_off = size - 1;
			}
			if(z_off < 0){
				z_off = 0;
			} else if(z_off > size - 1){
				z_off = size - 1;
			}
			y_off = ground_height(x_off, z_off)+player_height;
		}
	}
	if(keys[SDLK_d]){
		if(flying){
			x_off -= cos(PI*h_angle/180.0)*speed;
			z_off -= sin(PI*h_angle/180.0)*speed;
		} else {
			x_off -= cos(PI*h_angle/180.0)*speed;
			z_off -= sin(PI*h_angle/180.0)*speed;
			if(x_off < 0){
				x_off = 0;
			} else if(x_off > size - 1){
				x_off = size - 1;
			}
			if(z_off < 0){
				z_off = 0;
			} else if(z_off > size - 1){
				z_off = size - 1;
			}
			y_off = ground_height(x_off, z_off)+player_height;
		}
	}
	if(keys[SDLK_r]){
		if(flying){
			y_off += speed;
		}
	}
	if(keys[SDLK_f]){
		if(flying){
			y_off -= speed;
		}
	}
	if(keys[SDLK_j]){
		h_angle -= 4*speed;
	}
	if(keys[SDLK_l]){
		h_angle += 4*speed;
	}
	if(keys[SDLK_i]){
		v_angle -= 4*speed;
	}
	if(keys[SDLK_k]){
		v_angle += 4*speed;
	}
	if(keys[SDLK_z]){
		max_terrain_height = (GLfloat)size/max_height_div;
		current_heightmap = perlin_noise(power,power,.5,0,power-1);
		current_heightmap = normalize(current_heightmap, size, size, max_terrain_height);
		current_normalmap = make_normalmap(current_heightmap,size,size);
		make_textures();
		make_terrain();
		make_treemap(tree_seed);
		if(!flying){
			y_off = ground_height(x_off, z_off)+player_height;
		}
	}
	if(keys[SDLK_o]){
		//oceanify(current_heightmap, size, size, 0.1);
		//current_normalmap = make_normalmap(current_heightmap,size,size);
	}
	if(keys[SDLK_h]){
		hillify(current_heightmap, size, size, hill_factor);
		max_terrain_height = (GLfloat)pow((GLfloat)max_terrain_height,(GLfloat)hill_factor);
		current_normalmap = make_normalmap(current_heightmap,size,size);
		make_terrain();
		make_treemap(tree_seed);
		if(!flying){
			y_off = ground_height(x_off, z_off)+player_height;
		}
	}
	if(keys[SDLK_y]){
		hillify(current_heightmap, size, size, 1.0/hill_factor);
		max_terrain_height = (GLfloat)pow((GLfloat)max_terrain_height,(GLfloat)1.0/hill_factor);
		current_normalmap = make_normalmap(current_heightmap,size,size);
		make_terrain();
		make_treemap(tree_seed);
		if(!flying){
			y_off = ground_height(x_off, z_off)+player_height;
		}
	}
	if(keys[SDLK_g]){
		smoothify(current_heightmap, size, size, .9);
		current_normalmap = make_normalmap(current_heightmap,size,size);
		make_terrain();
		make_treemap(tree_seed);
		if(!flying){
			y_off = ground_height(x_off, z_off)+player_height;
		}
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
	if(keys[SDLK_n]){
		flying = !flying;
	}
	Uint8 key;
	for(key = SDLK_1; key <= SDLK_9; key++){
		if(keys[key]){
			power = key - SDLK_0;
			size = (int)pow((GLfloat)2,power);
			max_terrain_height = (GLfloat)size/max_height_div;
			current_heightmap = perlin_noise(power,power,.5,0,power-1);
			current_heightmap = normalize(current_heightmap, size, size, max_terrain_height);
			current_normalmap = make_normalmap(current_heightmap,size,size);
			make_textures();
			make_terrain();
			make_treemap(tree_seed);
			if(!flying){
				y_off = ground_height(x_off, z_off)+player_height;
			}
			fog_amount = pow(fog_decay,power);
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
			if(x_off < 0){
				x_off = 0;
			} else if(x_off > size - 1){
				x_off = size - 1;
			}
			if(z_off < 0){
				z_off = 0;
			} else if(z_off > size - 1){
				z_off = size - 1;
			}
			y_off = ground_height(x_off, z_off)+player_height;
			light_pos[1] = size/2;
		}
	}
}

GLfloat **d1_to_d2(GLfloat *one, int w, int h){
// obsolete. used back when I was using multidimensional arrays
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
	max_terrain_height = (GLfloat)size/max_height_div;
	//current_heightmap = make_terramap(power,.25); // Old way
	current_heightmap = perlin_noise(power,power,.5,0,power-1); // new way!
	current_heightmap = normalize(current_heightmap, size, size, max_terrain_height);
	current_normalmap = make_normalmap(current_heightmap,size,size);
	y_off = ground_height(0, 0)+player_height;

	SDL_Surface *screen;

	SDL_Init(SDL_INIT_VIDEO);

	int cur_screen_w = SCREEN_W, cur_screen_h = SCREEN_H;

	screen = SDL_SetVideoMode(cur_screen_w, cur_screen_h, SCREEN_BPP, SDL_OPENGL|SDL_RESIZABLE);

	if(screen == NULL){
		cout << "Could not initialize screen." << endl;
		exit(1);
	}

	SDL_WM_SetCaption("Terrain!", NULL);

	GLenum err = glewInit();
	if(err != GLEW_OK){
		cout << "Error: " << glewGetErrorString(err) << endl;
	}

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

	light_pos[1] = size/2; // nice y value

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_textures);
	cout << "max multitextures: " << max_textures << endl;

	make_textures();

	terrain_dl = glGenLists(1);

	make_terrain();
	
	tree_seed = rand();
	make_treemap(tree_seed);
	
	SDL_WM_GrabInput(SDL_GRAB_ON);

	SDL_ShowCursor(SDL_DISABLE);

	int frames = 0;
	int start = SDL_GetTicks();

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
		// handle input
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

		pressed = SDL_GetRelativeMouseState(&x, &y);

		if(!(pressed&SDL_BUTTON(2))){
			h_angle += (GLfloat)x;
			v_angle += (GLfloat)y;
			if(v_angle < -90) v_angle = -90;
			if(v_angle > 90) v_angle = 90;
		}

		keyboard(keys);

		draw_scene();

		SDL_GL_SwapBuffers();
		frames++;
		if(frames >= 250){
			int millis = SDL_GetTicks() - start;
			cout << (1000 * frames) / millis << " FPS" << endl;
			frames = 0;
			start = SDL_GetTicks();
		}
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1,&cloud_texture);
	glDeleteTextures(1,&tree_texture);
	glDeleteTextures(TEXTURE_COUNT,terrain_textures);

	glDeleteLists(terrain_dl,1);
	
	glDeleteLists(treemap_dl,1);

	delete[] current_heightmap;

	delete[] treemap;

	for(int i = 0; i < size*size; i++){
		delete[] current_normalmap[i];
	}
	delete[] current_normalmap;

	SDL_WM_GrabInput(SDL_GRAB_OFF);

	SDL_FreeSurface(screen);

	SDL_Quit();
	return 0;
}
