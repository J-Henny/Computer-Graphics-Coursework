/**
 * Jack Hurd
 * CSCI 4565
 * Homework 2
 * 2/28/22
 * 
 * This program handles viewing and modeling transformations
 * using openGL. It displays a teapot which can be 
 * viewed from a radius of 10 units away. An option to toggle
 * gridview is also available. 
 * 
 * 
 */
#include <stdlib.h>
#include <GL/glut.h>
#define TOGGLE_GRID 1
#define EXIT 2
#define WINDOWSIZE 500


GLint toggle = 0;
GLint gridGap = 2;

int x_1 = 0, x_2 = 0, y_1 = 0, y_2 = 0;

GLfloat elevation = 0.0; // some change in y
GLfloat swing = 0.0; // some change in x

GLboolean isSwing;
GLboolean isElevation;
GLboolean DRAG_RIGHT; // some positive change in x
GLboolean DRAG_LEFT; // some negative change in x
GLboolean DRAG_UP; // some positive change in y
GLboolean DRAG_DOWN; // some negative change in y
GLboolean isGrid;



/*  Initialize light source and lighting model.
 */
void 
myinit(void)
{
    GLfloat light_ambient[] =
    {0.0, 0.0, 0.0, 1.0};
    GLfloat light_diffuse[] =
    {1.0, 1.0, 1.0, 1.0};
    GLfloat light_specular[] =
    {1.0, 1.0, 1.0, 1.0};
/* light_position is NOT default value */
    GLfloat light_position[] =
    {1.0, 1.0, 0.0, 0.0};
    GLfloat global_ambient[] =
    {0.75, 0.75, 0.75, 1.0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

    glFrontFace(GL_CW);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glPointSize(5.0);
    glLogicOp(GL_XOR);
}

void
drawGrid(void)
{

    /*
    This function displays the grid with the teapot, if selected.
    */
    
    glColor3f(1.0, 1.0, 1.0);
    glEnable(GL_COLOR_LOGIC_OP);
    glBegin(GL_LINES);


    // draw panel along z axis

    // Vertical lines
    for (int z = -10; z < 10; z = z + gridGap) {
        glVertex3f(0.0, -10,0, z);
        glVertex3f(0.0, 10.0, z);
    }
    // Horitzontal lines
    for (int y = -10; y < 10; y = y + gridGap) {
        glVertex3f(0.0, y, -10.0);
        glVertex3f(0.0, y, 10.0);
    }

    // draw panel along x axis

    // Vertical lines
    for (int x = -10.0; x < 10.0; x = x + gridGap) {
        glVertex3f(x, -10.0, 0.0);
        glVertex3f(x, 10.0, 0.0);
    }
    // Horitzontal lines
    for (int y = -10; y < 10; y = y + gridGap) {
        glVertex3f(-10.0, y, 0.0);
        glVertex3f(10.0, y, 0.0);
    }

    glEnd();
    glFlush();
    glDisable(GL_COLOR_LOGIC_OP);
}

void 
display(void)
{
    GLfloat low_ambient[] =
    {0.1, 0.1, 0.1, 1.0};
    GLfloat more_ambient[] =
    {0.4, 0.4, 0.4, 1.0};
    GLfloat most_ambient[] =
    {1.0, 1.0, 1.0, 1.0};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    /*  material has moderate ambient reflection */
    glMaterialfv(GL_FRONT, GL_AMBIENT, more_ambient);
    glPushMatrix();

    glTranslatef(0.0, 0.0, -10.0); // translate object

    glRotatef((GLfloat)elevation, 1.0, 0.0, 0.0); // rotations about x
    glRotatef((GLfloat)swing, 0.0, 1.0, 0.0); // rotations about y

    if (isGrid) {
        drawGrid(); // draw grid if selected
   
    }
    
    
    glutSolidTeapot(1.0); // draw teapot first, working backwards here.
    glLoadIdentity();
    

   
    
    glPopMatrix();

    glFlush();

}

void processMenuEvents(int option) {
    /*
    This function processes the menu options the user inputs.
    */
    switch (option) {
    case TOGGLE_GRID:
        toggle += 1;
        if (toggle % 2 == 1)
        {
            isGrid = GL_TRUE;
        }
        else {
            isGrid = GL_FALSE;
        }


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

void 
myReshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30, (GLfloat)w/(GLfloat)h , 1, 30.0);
    glMatrixMode(GL_MODELVIEW);
}

/*  Main Loop
 *  Open window with initial window size, title bar, 
 *  RGBA display mode, and handle input events.
 */
int
main(int argc, char **argv)
{

    int menu;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow(argv[0]);
    myinit();
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutMotionFunc(motion);

    menu = glutCreateMenu(processMenuEvents); // creating menu object

    glutAddMenuEntry("Toggle Grid", TOGGLE_GRID); // adding entries
    glutAddMenuEntry("Exit", EXIT);

    glutAttachMenu(GLUT_RIGHT_BUTTON); // bind right button to menu.


    glutMainLoop();
    return 0;             /* ANSI C requires main to return int. */
}
