/**
 *CSCI 4565 
 * Jack Hurd
 * Due Date: 4/20/22
 *
 * This application demonstrates Cube environment mapping with mulitple shapes
 * and images. It supports fullscreen antialiasing and swing/elevation UI.
 * 
 */
#include <vector>
#include <GL/glut.h>
#include "jitter.h" // used for anti-aliasing.



extern "C" {
	#include "tga.h"
}

struct Vector3f {
	Vector3f(float x, float y, float z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	float x, y, z;
};

/**
 * Used to declare the constants related to the cube map in OpenGL.
 */
#ifndef GL_EXT_texture_cube_map
# define GL_NORMAL_MAP_EXT                   0x8511
# define GL_REFLECTION_MAP_EXT               0x8512
# define GL_TEXTURE_CUBE_MAP_EXT             0x8513
# define GL_TEXTURE_BINDING_CUBE_MAP_EXT     0x8514
# define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT  0x8515
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT  0x8516
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT  0x8517
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT  0x8518
# define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT  0x8519
# define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT  0x851A
# define GL_PROXY_TEXTURE_CUBE_MAP_EXT       0x851B
# define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT    0x851C
#endif

const char* WINDOW_TITLE = "[Example Environment (cube) Mapping]";
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 400;
GLint g_glutWindowIdentifier;

static GLenum targets[6] = {
  GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT
};

bool ortho = false;
const GLdouble FOV = 65.0;
const GLdouble NEAR_PLANE = 0.1;
const GLdouble FAR_PLANE = 10.0;

/**
 * LookAt Vectors - eye, look_at, and up
 */
Vector3f eye = Vector3f(0.0f, 0.0f, 5.0f);
Vector3f look_at = Vector3f(0.0f, 0.0f, 0.0f);
Vector3f up = Vector3f(0.0f, 1.0f, 0.0f);

/**
 * The mouse and rotation variables hold the state of the mouse coordinate
 * differences as it is dragged. The rotation of the displayed icosahedron 
 * depends on the dragging movement of the mouse.
 */
int mouse_cur[2] = {0, 0};
int mouse_last[2] = {0, 0};
float rotation_scale = 1.2f;
float rotation_angle_x = 0.0f;
float rotation_angle_y = 0.0f;

char *cube_images[6] =
{
  "cm_left.tga", 
  "cm_right.tga", 
  "cm_top.tga", 
  "cm_bottom.tga", 
  "cm_back.tga", 
  "cm_front.tga", 
};

char cube_image_own1[30] = "tyler.tga"; // our other tga mappings to use
char cube_image_own2[30] = "example1.tga";


enum {
	ORIGINAL, TYLER, SUNSET
};

bool mipmaps = true;
int wrap = GL_REPEAT;
bool smooth = true;
bool cube_map_enabled = true;
int toggleNum = 0;
bool environmentMapped = true;
int cube_map_mode = GL_NORMAL_MAP_EXT;

float angle_light_0 = 0.0f;
float light_0_dist = 3.0f;
GLfloat light_0_pos[] = { light_0_dist * cos(angle_light_0), 0.0f, light_0_dist * sin(angle_light_0), 0.0f };
GLfloat light_0_ambient[] = {0.1f, 0.2f, 0.4f, 1.0f};
GLfloat light_0_diffuse[] = {0.1f, 0.2f, 0.8f, 1.0f};
GLfloat light_0_specular[] = {0.0f, 0.0f, 1.0f, 1.0f};

float angle_light_1 = 9.6f;
float light_1_dist = 3.0f;
GLfloat light_1_pos[] = { light_1_dist * cos(angle_light_1), 0.0f, light_1_dist * sin(angle_light_1), 0.0f };
GLfloat light_1_ambient[] = {0.4f, 0.2f, 0.1f, 1.0f};
GLfloat light_1_diffuse[] = {0.8f, 0.2f, 0.1f, 1.0f};
GLfloat light_1_specular[] = {1.0f, 0.0f, 0.0f, 1.0f};

bool lighting = false;
bool light_0_enabled = true;
bool light_1_enabled = true;

void g_create_lights() {
	glLightfv(GL_LIGHT0, GL_POSITION, light_0_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_0_specular);

	glLightfv(GL_LIGHT1, GL_POSITION, light_1_pos);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_1_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_1_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_1_specular);
}

void g_load_texture(GLenum target, char* filename) {
	FILE* file;
	gliGenericImage* image;

	file = fopen(filename, "rb");
	if ( file == NULL ) exit(1);

	image = gliReadTGA(file, filename);
	fclose(file);

	if ( mipmaps == true )
		gluBuild2DMipmaps(target, image->components, image->width, image->height, image->format, GL_UNSIGNED_BYTE, image->pixels);
	else
		glTexImage2D(target, 0, image->components, image->width, image->height, 0, image->format, GL_UNSIGNED_BYTE, image->pixels);
}

void g_update_texgen() {
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, cube_map_mode);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, cube_map_mode);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, cube_map_mode);
}


void g_update_wrap() {
	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, wrap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, wrap);
}

void g_construct_cube_map(int image) {

	// This switch statement determines the image to be used for the mapping.
	switch (image) {
	case ORIGINAL:
		for (int i = 0; i < 6; i++) {
			g_load_texture(targets[i], cube_images[i]);
		}
		break;
	case TYLER:
		for (int i = 0; i < 6; i++) {
			g_load_texture(targets[i], cube_image_own1);
		}
		break;
	case SUNSET:
		for (int i = 0; i < 6; i++) {
			g_load_texture(targets[i], cube_image_own2);
		}
		break;
	}

	

	if ( mipmaps == true ) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glEnable(GL_TEXTURE_CUBE_MAP_EXT);
	
	g_update_texgen();
	g_update_wrap();

	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);
	glEnable(GL_TEXTURE_GEN_R);
}

void g_init() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearAccum(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	g_create_lights();
	g_construct_cube_map(ORIGINAL);

}

void g_glutReshapeFunc(int width, int height) {
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(FOV, ((GLfloat)width/(GLfloat)height), NEAR_PLANE, FAR_PLANE);
	gluLookAt(eye.x, eye.y, eye.z, look_at.x, look_at.y, look_at.z, up.x, up.y, up.z);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glutPostRedisplay();
}

void g_draw_lights() {
	if ( lighting == false ) return;
	glDisable(GL_TEXTURE_CUBE_MAP_EXT);
	glDisable(GL_LIGHTING);
	glPointSize(6.0f);
	glBegin(GL_POINTS);
		glColor3f(light_0_diffuse[0], light_0_diffuse[1], light_0_diffuse[2]);
		glVertex3f(light_0_pos[0], light_0_pos[1], light_0_pos[2]);
		glColor3f(light_1_diffuse[0], light_1_diffuse[1], light_1_diffuse[2]);
		glVertex3f(light_1_pos[0], light_1_pos[1], light_1_pos[2]);
	glEnd();
	glEnable(GL_LIGHTING);
	if (cube_map_enabled)
		glEnable(GL_TEXTURE_CUBE_MAP_EXT);
}

enum {
	NONE, AA
};
enum {
	TEAPOT, TORUS, ICOSAHEDRON
};

int rendermode = NONE;

int shape = TEAPOT;

void drawShape(int shape) {
	// This function decides which shape to draw.
	glPushMatrix();
	glRotatef(rotation_angle_x, 0.0f, 1.0f, 0.0f);
	glRotatef(rotation_angle_y, 1.0f, 0.0f, 0.0f);
	switch (shape) {
	case TEAPOT:
		glutSolidTeapot(1.5);
		break;
	case TORUS:
		glutSolidTorus(0.275, 0.85, 10, 15);
		break;
	case ICOSAHEDRON:
		glutSolidIcosahedron();
		break;
	}

	glPopMatrix();
	g_draw_lights();
}



int texture = ORIGINAL;

void
textureSelect(int select) {

	g_construct_cube_map(select);
	glutPostRedisplay();

}

void
shapeSelect(int value) {
	shape = value;

	glutPostRedisplay();
}

void menu(int selection)
{
	rendermode = selection;

	glutPostRedisplay();

}
#define ACSIZE 8
void g_glutDisplayFunc() {
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	int i, j;
	int min, max;
	int count;
	GLfloat invx, invy;
	GLfloat scale, dx, dy;

	switch (rendermode) {

		// This determines whether antialiasing or regular rendering is selected.
	case NONE:
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(FOV, ((GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT), NEAR_PLANE, FAR_PLANE);
		gluLookAt(eye.x, eye.y, eye.z, look_at.x, look_at.y, look_at.z, up.x, up.y, up.z);
		glMatrixMode(GL_MODELVIEW);
		drawShape(shape);

		break;
	case AA:
		GLint viewport[4];
		int jitter;

		glGetIntegerv(GL_VIEWPORT, viewport);

		// Using jitter.h to calculate jitter for each point on shape.

		glClear(GL_ACCUM_BUFFER_BIT);
		for (jitter = 0; jitter < ACSIZE; jitter++) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glPushMatrix();
			/*	Note that 4.5 is the distance in world space between
			 *	left and right and bottom and top.
			 *	This formula converts fractional pixel movement to
			 *	world coordinates.
			 */
			glTranslatef(j8[jitter].x * 4.5 / viewport[2],
				j8[jitter].y * 4.5 / viewport[3], 0.0);
			drawShape(shape);
			glPopMatrix();
			glAccum(GL_ACCUM, 1.0 / ACSIZE);
		}
		glAccum(GL_RETURN, 1.0);
		break;

	}
	
	glutSwapBuffers();
}

void g_glutKeyboardFunc(unsigned char key, int x, int y) {
	if ( key == 'j' ) { angle_light_0 += 0.2f; }
	if ( key == 'k' ) { angle_light_0 -= 0.2f; }
	if ( key == 'u' ) { angle_light_1 += 0.2f; }
	if ( key == 'i' ) { angle_light_1 -= 0.2f; }

	if ( key == 'r' ) { 
		cube_map_mode = GL_NORMAL_MAP_EXT;
		g_update_texgen();
	}

	if ( key == 't' ) { 
		cube_map_mode = GL_REFLECTION_MAP_EXT;
		g_update_texgen();
	}
	if (key == 'e') {
		cube_map_mode = GL_OBJECT_LINEAR;
		g_update_texgen();
	}

	light_0_pos[0] = light_0_dist * cos(angle_light_0);
	light_0_pos[2] = light_0_dist * sin(angle_light_0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_0_pos);

	light_1_pos[0] = light_1_dist * cos(angle_light_1);
	light_1_pos[2] = light_1_dist * sin(angle_light_1);
	glLightfv(GL_LIGHT1, GL_POSITION, light_1_pos);

	glutPostRedisplay();
}

void g_glutMotionFunc(int x, int y) {
	mouse_cur[0] = x;
	mouse_cur[1] = y;

	int x_diff = mouse_last[0] - mouse_cur[0];
	int y_diff = mouse_last[1] - mouse_cur[1];

	rotation_angle_x += rotation_scale * (float)x_diff;
	rotation_angle_y += rotation_scale * (float)y_diff;

	mouse_last[0] = mouse_cur[0];
	mouse_last[1] = mouse_cur[1];
	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	int shapeMenu, imageMenu;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	g_glutWindowIdentifier = glutCreateWindow(WINDOW_TITLE);

	glutDisplayFunc(g_glutDisplayFunc);
	glutReshapeFunc(g_glutReshapeFunc);
	glutKeyboardFunc(g_glutKeyboardFunc);
	glutMotionFunc(g_glutMotionFunc);

#define SHAPE_ENTRIES() \
	glutAddMenuEntry("Teapot", TEAPOT); \
	glutAddMenuEntry("Torus", TORUS); \
	glutAddMenuEntry("Icosahedron", ICOSAHEDRON); \

#define IMAGE_ENTRIES() \
	glutAddMenuEntry("Original", ORIGINAL); \
	glutAddMenuEntry("Tyler", TYLER); \
	glutAddMenuEntry("Sunset", SUNSET); \

	// menus for selecting the shape and image.
	e
	shapeMenu = glutCreateMenu(shapeSelect);
	SHAPE_ENTRIES();

	imageMenu = glutCreateMenu(textureSelect);
	IMAGE_ENTRIES();

	glutCreateMenu(menu);
	glutAddMenuEntry("Aliased View", NONE);
	glutAddMenuEntry("AntiAliased", AA); // selecting antialiasing
	glutAddSubMenu("Change Shape", shapeMenu);
	glutAddSubMenu("Change Image", imageMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);



	g_init();

	glutMainLoop();
	return 0;
}