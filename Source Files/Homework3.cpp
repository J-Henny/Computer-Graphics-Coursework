/**
 * Jack Hurd
 * CSCI 4565
 * Homework 3
 * 3/22/22
 *
 * This program handles subdivisions of polygons of an isocahedron
 * and lighting. It uses the swing and elevation UI from homework 2, and has
 * the ability to toggle the light.
 *
 *
 */
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#define FIRST_SUB 1
#define SECOND_SUB 2
#define THIRD_SUB 3
#define FOURTH_SUB 4
#define TOGGLE_LIGHT 5
#define FLAT_SHADING 6
#define SMOOTH_SHADING 7
#define EXIT 8
#define WINDOWSIZE 500
#define X .525731112119133606 
#define Z .850650808352039932


int x_1 = 0, x_2 = 0, y_1 = 0, y_2 = 0;
int sub = 0;
GLint toggle = 0;

GLfloat elevation = 0.0; // some change in y
GLfloat swing = 0.0; // some change in x

GLboolean isSwing;
GLboolean isElevation;
GLboolean DRAG_RIGHT; // some positive change in x
GLboolean DRAG_LEFT; // some negative change in x
GLboolean DRAG_UP; // some positive change in y
GLboolean DRAG_DOWN; // some negative change in y

enum{LIGHT_OFF, LIGHT_RED, LIGHT_WHITE, LIGHT_GREEN} LightValues;


GLfloat white_light[] =
{ 1.0, 1.0, 1.0, 1.0 };
GLfloat left_light_position[] =
{ -10.0, 0.0, -10.0, 0.0 }, right_light_position[] =
{ 10.0, 0.0, -10.0, 0.0 };




int shade_model = GL_FLAT;
char* left_light, * right_light;

void init(void)
{

    /*
    * This function handles initializing the lights and material.
    
    */
    GLfloat mat_ambient[] = { 1.0, 0.0, 0.0, 1.0 };
    

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(shade_model);
    glEnable(GL_DEPTH_TEST);
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
    glLightf(GL_LIGHT0, GL_POSITION, *left_light_position); // setting left light
    glLightf(GL_LIGHT1, GL_POSITION, *right_light_position); // setting right light
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
}



void normalize(float v[3]) {

    /*
    This function normalizes the vector passed into it.
    */
    GLfloat d = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (d == 0.0) {
        perror("zero length vector");
        return;
    }
    v[0] /= d; v[1] /= d; v[2] /= d;
}

void normcrossprod(float v1[3], float v2[3], float out[3])
{
    /*
    This function calculates the normal cross product of the vectors passed in.*/
    GLint i, j;
    GLfloat length;

    out[0] = v1[1] * v2[2] - v1[2] * v2[1];
    out[1] = v1[2] * v2[0] - v1[0] * v2[2];
    out[2] = v1[0] * v2[1] - v1[1] * v2[0];
    normalize(out);
}

void drawtriangle(float* v1, float* v2, float* v3)
{
    /*
    This function draws each triangle for the isocahedron*/
    glBegin(GL_POLYGON);
    glNormal3fv(v1); glVertex3fv(v1); // every vertex is normalized for lighting purposes.
    glNormal3fv(v2); glVertex3fv(v2);
    glNormal3fv(v3); glVertex3fv(v3);
    glEnd();
}

void subdivide(float* v1, float* v2, float* v3, long depth)
{

    /*
    * This function performs the recursive subdivision of the isocahedron into its desired number of triangles.
    */
    GLfloat v12[3], v23[3], v31[3];
    GLint i;

    if (depth == 0) {
        drawtriangle(v1, v2, v3);
        return;
    }
    for (i = 0; i < 3; i++) {
        v12[i] = (v1[i] + v2[i]) / 2;
        v23[i] = (v2[i] + v3[i]) / 2;
        v31[i] = (v3[i] + v1[i]) / 2;
    }
    normalize(v12);
    normalize(v23);
    normalize(v31);
    subdivide(v1, v12, v31, depth - 1); // recursively calling until we reach the base case
    subdivide(v2, v23, v12, depth - 1);
    subdivide(v3, v31, v23, depth - 1);
    subdivide(v12, v23, v31, depth - 1);
}

void
display(void)
{
    /*
    * This function displays the icosahedron handles how deep the subdivision should go, based on user selection.
    */


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glShadeModel(shade_model);

    glPushMatrix();

    glTranslatef(0.0, 0.0, -10.0); // translate object

    glRotatef((GLfloat)elevation, 1.0, 0.0, 0.0); // rotations about x
    glRotatef((GLfloat)swing, 0.0, 1.0, 0.0); // rotations about y


    static GLfloat vdata[12][3] = {
       {-X, 0.0, Z}, {X, 0.0, Z}, {-X, 0.0, -Z}, {X, 0.0, -Z}, // indicies for beginning model
       {0.0, Z, X}, {0.0, Z, -X}, {0.0, -Z, X}, {0.0, -Z, -X},
       {Z, X, 0.0}, {-Z, X, 0.0}, {Z, -X, 0.0}, {-Z, -X, 0.0}
    };

    static GLint tindices[20][3] = {
       {0,4,1}, {0,9,4}, {9,5,4}, {4,5,8}, {4,8,1}, // pairing the vertices for each triangle drawing
       {8,10,1}, {8,3,10}, {5,3,8}, {5,2,3}, {2,7,3},
       {7,10,3}, {7,6,10}, {7,11,6}, {11,0,6}, {0,1,6},
       {6,1,10}, {9,0,11}, {9,11,2}, {9,2,5}, {7,2,11} };

    for (int i = 0; i < 20; i++) {
        GLfloat d1[3], d2[3], norm[3];
        for (int j = 0; j < 3; j++) {
            d1[j] = vdata[tindices[i][0]][j] - vdata[tindices[i][1]][j];
            d2[j] = vdata[tindices[i][1]][j] - vdata[tindices[i][2]][j];
        }
        normcrossprod(d1, d2, norm);
        glNormal3fv(norm);
        subdivide(&vdata[tindices[i][0]][0], // calling subdivide to divide based on user input
            &vdata[tindices[i][1]][0],
            &vdata[tindices[i][2]][0], sub);
        
   
        
        
    }
    glLoadIdentity();



    glPopMatrix();

    glFlush();

}



void
myReshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30, (GLfloat)w / (GLfloat)h, 1, 30.0);
    glMatrixMode(GL_MODELVIEW);
}


void processMenuEvents(int option) {
    /*
    This function processes the menu options the user inputs.
    */
    switch (option) {
    case FIRST_SUB:
        sub = 0;
        break;
    case SECOND_SUB:
        sub = 1;
        break;
    case THIRD_SUB:
        sub = 2;
        break;
    case FOURTH_SUB:
        sub = 3;
        break;
    case TOGGLE_LIGHT:
        toggle += 1;
        if (toggle % 2 == 1)
        {
            glDisable(GL_LIGHT0);
        }
        else {
            glEnable(GL_LIGHT0);
        }
        break;
    case FLAT_SHADING:
        shade_model = GL_FLAT;
        break;
    case SMOOTH_SHADING:
        shade_model = GL_SMOOTH;
        break;
    case EXIT:
        exit(1);
        break;
    }

    glutPostRedisplay();
}

void
motion(int mouse_x, int mouse_y) {

    /*
    * We use the motion function to handle the change in view of the object.
    */
    x_2 = mouse_x; // new drag coords
    y_2 = WINDOWSIZE - mouse_y;

    int difference_x = x_2 - x_1;
    int difference_y = y_2 - y_1;

    isSwing = abs(difference_x) > 0;
    isElevation = abs(difference_y) > 0;

    if (isSwing) {
        DRAG_RIGHT = difference_x > 0;
        DRAG_LEFT = difference_x < 0;
        if (DRAG_RIGHT) {
            swing += 3.0;
        }
        else if (DRAG_LEFT) {

            swing -= 3.0;
        }

    }
    if (isElevation) {
        DRAG_UP = difference_y > 0;
        DRAG_DOWN = difference_y < 0;
        if (DRAG_UP) {

            elevation += 3.0;
        }
        else if (DRAG_DOWN) {

            elevation -= 3.0;
        }
    }

    x_1 = x_2; // Prepare for another change in x and y values
    y_1 = y_2;
    glutPostRedisplay();
}

int
main(int argc, char** argv)
{
    /*
    * This function is the main loop where all events are called.
    */
    int menu;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow(argv[0]);
    init();
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutMotionFunc(motion);

    menu = glutCreateMenu(processMenuEvents); // creating menu object

    glutAddMenuEntry("20 Triangles", FIRST_SUB); // adding entries
    glutAddMenuEntry("80 Triangles", SECOND_SUB);
    glutAddMenuEntry("320 Triangles", THIRD_SUB);
    glutAddMenuEntry("1280 Triangles", FOURTH_SUB);
    glutAddMenuEntry("Toggle Light", TOGGLE_LIGHT);
    glutAddMenuEntry("Flat Shading", FLAT_SHADING);
    glutAddMenuEntry("Smooth Shading", SMOOTH_SHADING);
    glutAddMenuEntry("Exit", EXIT);

    glutAttachMenu(GLUT_RIGHT_BUTTON); // bind right button to menu.

  

    glutMainLoop();
    return 0;             /* ANSI C requires main to return int. */
}
