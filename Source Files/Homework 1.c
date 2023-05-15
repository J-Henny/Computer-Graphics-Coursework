/**
Jack Hurd
CSCI 4565
2/15/22
Homework 1 


This program focuses on implementing display, mouse, 
and motion functions in openGL. The user may click
the left side of the mouse to create a point, and
freely drag around the mouse displaying a line 
at the current position. New points will be added each
time the user clicks. Once the right side of the mouse is clicked,
the lines complete and display a polygon.


**/


#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const GLint WINDOW_HEIGHT = 500;
const GLint WINDOW_WIDTH = 500;
int Vertices [80];
GLint g_nVertices = 0;
GLboolean Dragging = GL_FALSE;
GLboolean PolyComplete = GL_FALSE;
GLboolean Restart = GL_FALSE;
int i = 0;

void displayExistingVertices() {
    /**
    * This function displays each vertex anytime
    * the display function is called.
    **/
    for (int j = 0; j < i; j += 2)
    {
        glColor3f(1.0, 0.0, 0.0);
        glEnable(GL_COLOR_LOGIC_OP);
        glBegin(GL_POINTS);
        glVertex2i(Vertices[j], Vertices[j + 1]);
        glEnd();
        glFlush();
        glDisable(GL_COLOR_LOGIC_OP);
    }
}
void displayExistingLines() {
    /**
    * This function displays each line anytime
    * the display function is called.
    **/
    for (int j = 0; j < i - 2; j += 2) 
        // Must iterate only twice since the same vertex is used twice in a row.
    {
        glColor3f(1.0, 0.0, 1.0);
        glEnable(GL_COLOR_LOGIC_OP);
        glBegin(GL_LINES);
        glVertex2i(Vertices[j], Vertices[j + 1]);
        glVertex2i(Vertices[j + 2], Vertices[j + 3]);
        glEnd();
        glFlush();
        glDisable(GL_COLOR_LOGIC_OP);
    }

}
void drawPolygon()
{
    /**
    * This function displays the polygon once the 
    user clicks the left side of the mouse.
    **/
    if (PolyComplete == GL_TRUE) { // Only enter when mouse click flags this
        // Drawing the last line
        glColor3f(1.0, 0.0, 1.0);
        glEnable(GL_COLOR_LOGIC_OP);
        glBegin(GL_LINES);
        glVertex2i(Vertices[i - 2], Vertices[i - 1]);
        glVertex2i(Vertices[0], Vertices[1]); // Connect to first vertex
        glEnd();
        glFlush();
        glDisable(GL_COLOR_LOGIC_OP);


        // Add all other points stored in data structure.
        for (int j = 0; j < i; j += 2) {

            glColor3f(0.0, 0.0, 1.0);
            glEnable(GL_COLOR_LOGIC_OP);
            glBegin(GL_POLYGON);
            glVertex2i(Vertices[j], Vertices[j + 1]);
            
        }
        glEnd();
        glFlush();
        glDisable(GL_COLOR_LOGIC_OP);

        Restart = GL_TRUE; // Flag to clear display once clicked again

    }


}
void clear()
{
    /**
    * This function clears the display once the user
    * clicks the mouse after finishing the polygon.
    **/
    if (Restart == GL_TRUE)
    {
        for (int j = 0; j < i; j++)
        {
            Vertices[j] = 0; // Set each element to 0
        }
        i = 0;
        Restart = GL_FALSE;
        PolyComplete = GL_FALSE;
    }
    
    

}

void display(void)
{
    /**
    * This function displays all called functions for the main loop.
    **/
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glColor3f(1.0, 1.0, 1.0);
    displayExistingVertices(); // drawing vertices
    displayExistingLines(); // drawing lines
    drawPolygon(); // drawing polygon when flagged
    clear(); // clearing when flagged
    glPopMatrix();

    glutSwapBuffers();
}

void init(void)
{
    /**
    * This function is for environment initalization upon 
    * beginning the main loop.
    **/
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glPointSize(5.0);
    glLogicOp(GL_XOR);
    
}


void reshape(int w, int h)
{
    /**
    * This function is for assisting in the 
    * visual capability when redisplaying in
    * each loop.
    **/
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glutPostRedisplay();
}
void newLine(int mouse_x, int mouse_y)
{
    // Every time there is a new line, we want to call the display function to display the new line.
    glutPostRedisplay();

}

void newVertex(int mouse_x, int mouse_y)
{
    /**
    * This function sets the vertex variables
    * and notifies the display to be called to 
    * show the new vertex.
    **/
    g_nVertices += 2;
    int x = mouse_x;
    int y = WINDOW_HEIGHT - mouse_y;
    Vertices[i] = x;
    Vertices[i + 1] = y; // saving vertices into external data structure
    Vertices[i + 2] = x;// saving first line
    Vertices[i + 3] = y;
    i += 4; // new place for vertex to be stored
    glutPostRedisplay();
}


void onMouseMove(int mouse_x, int mouse_y)
{
    /**
    * This function updates the value of the endpoint
    * of the current line for displaying the real time dragging.
    **/
    Dragging = GL_TRUE;
    int x = mouse_x;
    int y = WINDOW_HEIGHT - mouse_y;

    Vertices[i - 2] = x;
    Vertices[i - 1] = y;
    glutPostRedisplay();
    Dragging = GL_FALSE;
}


void makePolygon() {
    //Calling the display function to display the polygon
    PolyComplete = GL_TRUE;
    glutPostRedisplay();
}
void clearScreen() {
    // Calling display function to clear the screen
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) 
{
    /**
    * This function handles the required mouse inputs.
    **/
   switch (button) {
      case GLUT_LEFT_BUTTON:
          if (state == GLUT_DOWN) {
              if (Restart == GL_TRUE)
              {
                  clearScreen(); // If true we clear the screen for this new set of lines
              }
              newVertex(x, y); // Else we are adding a new line and point
              newLine(x, y);
              Dragging = GL_TRUE;
          }
         break;
      case GLUT_MIDDLE_BUTTON:
      case GLUT_RIGHT_BUTTON:
          if (state == GLUT_DOWN) {
              makePolygon(); // Create the polygon when the right side is clicked.
          }
              
            glutIdleFunc(NULL);
         break;
      default:
         break;
   }
}




int main(int argc, char** argv)
{
    /**
    * This function is the main driver
    * calling all required mouse, motion and
    * display functions for glut's main loop.
    **/
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(250, 250);
    glutCreateWindow("double buffering");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMotionFunc(onMouseMove);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}
