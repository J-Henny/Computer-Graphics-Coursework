/*
* Jack Hurd
* CSCI 4565
* Final Project
* Due Date: 5/11/22
* 
* Description:
* 
* This program compiles several components of a 3D rendered scene, 
* including 3ds modeling, shading, texture mapping, rotations/transformations, lighting/material, etc.
* 
* It displays a hubble space telescope and features ability to manipulate material and lighting, textures,
* shading, etc.
* 
* Translations of the object can be made with the arrow keys, and pgup/pgdown for moving in the z axis.
* 
* Rotations can be made by dragging the mouse, using elevation/swing UI.
* 
* 
*/#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>


#include "textfile.h"
#include "mesh.h"
#include "texture.h"
#include "3dsloader.h"



int screen_width = 1000;
int screen_height = 1000;


GLint loc;
GLuint v, f, f2, p;
GLfloat elevation = 0.0; // some change in y
GLfloat swing = 0.0; // some change in x
GLboolean isSwing;
GLboolean isElevation;
GLboolean DRAG_RIGHT; // some positive change in x
GLboolean DRAG_LEFT; // some negative change in x
GLboolean DRAG_UP; // some positive change in y
GLboolean DRAG_DOWN; // some negative change in y
GLfloat light1_position[] = {-300.0, 0.0, -1300.0, 0.0 };/*rotate about x*/
GLfloat light2_position[] = { 300.0, 0.0, -1300.0, 0.0 }; /*rotate about y*/
GLfloat light3_position[] = { 0.0, 300.0, -1300.0, 0.0 }; /* rotate about z */
GLfloat red_light[] =
{ 1.0, 0.0, 0.0, 1.0 }, green_light[] =
{ 0.0, 1.0, 0.0, 1.0 }, white_light[] =
{ 1.0, 1.0, 1.0, 1.0 };

enum mainMenuOptions {
    LIGHTS, MATERIAL, SHADERS, TEXTURES
};
enum textureOptions {
    TEXTURE1, TEXTURE2, TEXTURE3, TEXTURE4, TEXTURE5
};
enum shadingOptions {
    NORMAL, PERPIXEL, PHONG, TOON, GLASS
};
enum RGBOptions {
    R, G, B, A
};
enum lightOptions {
    ON, OFF
};

int shadeSelect = PERPIXEL;
int RGBSelect = R;
int textureSelect = TEXTURE1;
int initFlag = 1;



GLdouble high = 1.0, medium = 0.6, low = 0.2, off = 0.0;

GLint spin = 10.0;
GLint translate = 0.0;
/* Frag and vert files for shading. */
char normalVert[30] = "minimal.vert";
char normalFrag[30] = "minimal.frag";
char perPixelVert[30] = "perpixel_light.vert";
char perPixelFrag[30] = "perpixel_light.frag";
char phongVert[30] = "phong.vert";
char phongFrag[30] = "phong.frag";
char toonVert[30] = "toon.vert";
char toonFrag[30] = "toon.frag";
char glassVert[30] = "glass.vert";
char glassFrag[30] = "glass.frag";
GLfloat scale = 2.0;
int x_1 = 0, x_2 = 0, y_1 = 0, y_2 = 0;

// Absolute rotation values (0-359 degrees) and rotation increments for each frame
double translation_x = 0, translation_x_increment = 10.0;
double translation_y = 0, translation_y_increment = 10.0;
double translation_z = 0, translation_z_increment = 10.0;


GLfloat matAmbience[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat matDiffuse[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat matSpecular[] = { 0.0, 0.0, 0.0, 1.0};
GLfloat matShininess[] = { 50.0 };

 
// Flag for rendering as lines or filled polygons
int filling = 1;

Mesh mesh;
Texture texture, texture2, texture3, texture4, texture5;


void loadTexture() {
    // Load3DS(&mesh, "hst.3ds");
    /* This function handles which texture to load onto the object. */
    switch (textureSelect) {
    case TEXTURE1:
        texture.loadPNG("hbltel_1.png");
        mesh.id_texture = texture.texture_id;
        break;
    case TEXTURE2:
        texture2.loadPNG("hbltel_2.png");
        mesh.id_texture = texture2.texture_id;
        break;
    case TEXTURE3:
        texture3.loadPNG("hbltel_3.png");
        mesh.id_texture = texture3.texture_id;
        break;
    case TEXTURE4:
        texture4.loadPNG("hbltel_4.png");
        mesh.id_texture = texture4.texture_id;
        break;
    case TEXTURE5:
        texture5.loadPNG("hbltel_w.png");
        mesh.id_texture = texture5.texture_id;
        break;
    }
    
    if (initFlag == 1) {
        initFlag = -1;
    }
    else {
        glutPostRedisplay();
    }
    
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0); // This clear the background color to black
    glShadeModel(GL_SMOOTH); // Type of shading for the polygons
    glEnable(GL_DEPTH_TEST); // We enable the depth test (also called z buffer)
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbience);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    
    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glPolygonMode (GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)
    glEnable(GL_TEXTURE_2D);

    Load3DS(&mesh, "hst.3ds");

    loadTexture();

}

void resize(int width, int height) {
    screen_width = width;
    screen_height = height;
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,screen_width,screen_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)screen_width/(GLfloat)screen_height, 0.1f, 10000.0f);
    glutPostRedisplay();
}

void keyboard (unsigned char key, int x, int y) {  
    switch (key) {
        case 't': case 'T':
            if (filling==0) {
                glPolygonMode (GL_FRONT_AND_BACK, GL_FILL); // Polygon rasterization mode (polygon filled)
                filling=1;
            }   
            else {
                glPolygonMode (GL_FRONT_AND_BACK, GL_LINE); // Polygon rasterization mode (polygon outlined)
                filling=0;
            }
        break;
        /* This takes the object back to the origin */
        case '0':
            translation_x = 0.0;
            translation_y = 0.0;
            translation_z = 0.0;
    }
}

void keyboard_s (int key, int x, int y) {  
    /* The keyboard handles object translation. */
    switch (key) {
        case GLUT_KEY_UP:
            translation_y = translation_y - translation_y_increment;
        break;
        case GLUT_KEY_DOWN:
            translation_y = translation_y + translation_y_increment;
        break;
        case GLUT_KEY_LEFT:
            translation_x = translation_x - translation_x_increment;
        break;
        case GLUT_KEY_RIGHT:
            translation_x = translation_x + translation_x_increment;
        break;
        case GLUT_KEY_PAGE_UP:
            translation_z = translation_z + translation_z_increment;
        break;
        case GLUT_KEY_PAGE_DOWN:
            translation_z = translation_z - translation_z_increment;
            break;
        

    }
    
}


void
motion(int mouse_x, int mouse_y) {

    /*
    * We use the motion function to handle the change in view of the object.
    */
    x_2 = mouse_x; // new drag coords
    y_2 = screen_height - mouse_y;

    int difference_x = x_2 - x_1;
    int difference_y = y_2 - y_1;

    isSwing = abs(difference_x) > 0;
    isElevation = abs(difference_y) > 0;

    if (isSwing) {
        DRAG_RIGHT = difference_x > 0;
        DRAG_LEFT = difference_x < 0;
        if (DRAG_RIGHT) {
            swing += scale;
        }
        else if (DRAG_LEFT) {

            swing -= scale;
        }

    }
    if (isElevation) {
        DRAG_UP = difference_y > 0;
        DRAG_DOWN = difference_y < 0;
        if (DRAG_UP) {

            elevation += scale;
        }
        else if (DRAG_DOWN) {

            elevation -= scale;
        }
    }

    x_1 = x_2; // Prepare for another change in x and y values
    y_1 = y_2;
    glutPostRedisplay();
}

int printOglError(char* file, int line)
{
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten = 0;
    char* infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
        infoLog = (char*)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten = 0;
    char* infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0)
    {
        infoLog = (char*)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n", infoLog);
        free(infoLog);
    }
}

void setShaders() {
    /* This function loads in our shading programs and shader objects. */

    char* vs = NULL, * fs = NULL, * fs2 = NULL;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);
    f2 = glCreateShader(GL_FRAGMENT_SHADER);

    switch (shadeSelect) {
    case NORMAL:
        vs = textFileRead(normalVert);
        fs = textFileRead(normalFrag);
        break;
    case PERPIXEL:
        vs = textFileRead(perPixelVert);
        fs = textFileRead(perPixelFrag);
        break;
    case PHONG:
        vs = textFileRead(phongVert);
        fs = textFileRead(phongFrag);
        break;
    case TOON:
        vs = textFileRead(toonVert);
        fs = textFileRead(toonFrag);
        break;
    case GLASS:
        vs = textFileRead(glassVert);
        fs = textFileRead(glassFrag);
        
    }
    
    

    const char* vv = vs;
    const char* ff = fs;

    glShaderSource(v, 1, &vv, NULL);
    glShaderSource(f, 1, &ff, NULL);

    free(vs); free(fs);

    glCompileShader(v);
    glCompileShader(f);

    printShaderInfoLog(v);
    printShaderInfoLog(f);
    printShaderInfoLog(f2);

    p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);


    glLinkProgram(p);
    printProgramInfoLog(p);

    glUseProgram(p);
    loc = glGetUniformLocation(p, "time");

  


}


void mouse(int button, int state, int x, int y)
{
    switch (button) {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN) {
            spin = (spin + 30) % 360;
            glutPostRedisplay();
        }
        break;
    case GLUT_MIDDLE_BUTTON:
        if (state == GLUT_DOWN) {
            translate = translate + 50;
            glutPostRedisplay();
        }
    default:
        break;
    }
}





void mainMenuHandler(int selection) {
    if (selection == 666)
        exit(0);
    glutPostRedisplay();
}

void textureMenuHandler(int selection) {
    textureSelect = selection;
    loadTexture();
}

void shaderMenuHandler(int selection) {
    shadeSelect = selection;
    setShaders();
    glutPostRedisplay();
}



void lightMenuHandler(int selection) {
    switch (selection) {
    case ON:
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);
        break;
    case OFF:
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT1);
        glDisable(GL_LIGHT2);
        glDisable(GL_LIGHTING);
        break;
    }
    glutPostRedisplay();
}
void materialMenuHandler(int selection) {
    switch (selection) {
    case R:
        matDiffuse[0] = 1.0;
        matDiffuse[1] = 0.0;
        matDiffuse[2] = 0.0;
        break;
    case G:
        matDiffuse[0] = 0.0;
        matDiffuse[1] = 1.0;
        matDiffuse[2] = 0.0;
        break;
    case B:
        matDiffuse[0] = 0.0;
        matDiffuse[1] = 0.0;
        matDiffuse[2] = 1.0;
        break;
    }
    glutPostRedisplay();

}




void display() {

    
    
   

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glTranslatef(0.0,0.0,-1300.0);

    glTranslatef(translation_x, 0.0, 0.0); /* Translations of object */
    glTranslatef(0.0, translation_y, 0.0);
    glTranslatef(0.0, 0.0, translation_z);

    glRotatef((GLfloat)elevation, 1.0, 0.0, 0.0); // rotations about x
    glRotatef((GLfloat)swing, 0.0, 1.0, 0.0); // rotations about y
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse); /* update material info */
    /*glPushMatrix();
    glTranslated((GLdouble)translate, 0.0, 0.0);
    glRotated((GLdouble)spin, 1.0, 0.0, 0.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
    //glTranslated(0.0, -50.0, -1300.0);
    glDisable(GL_LIGHTING);
    glEnable(GL_LIGHTING);
    

    glPushMatrix();
    glTranslated(0.0, (GLdouble)translate, 0.0);
    glRotated((GLdouble)spin, 0.0, 1.0, 0.0);
    glLightfv(GL_LIGHT1, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, green_light);
    //glTranslated(0.0, -50.0, -1300.0);
    glDisable(GL_LIGHTING);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    glPopMatrix();
    */



 
    
    glBindTexture(GL_TEXTURE_2D, mesh.id_texture); // We set the active texture 

    glBegin(GL_TRIANGLES); // glBegin and glEnd delimit the vertices that define a primitive (in our case triangles)
    for ( int l_index = 0; l_index < mesh.polygons_qty; l_index++ ) {
        //----------------- FIRST VERTEX -----------------
        // Texture coordinates of the first vertex
        glTexCoord2f( mesh.mapcoord[ mesh.polygon[l_index].a ].u,
                      mesh.mapcoord[ mesh.polygon[l_index].a ].v);
        // Coordinates of the first vertex
        glVertex3f( mesh.vertex[ mesh.polygon[l_index].a ].x,
                    mesh.vertex[ mesh.polygon[l_index].a ].y,
                    mesh.vertex[ mesh.polygon[l_index].a ].z); //Vertex definition

        //----------------- SECOND VERTEX -----------------
        // Texture coordinates of the second vertex
        glTexCoord2f( mesh.mapcoord[ mesh.polygon[l_index].b ].u,
                      mesh.mapcoord[ mesh.polygon[l_index].b ].v);
        // Coordinates of the second vertex
        glVertex3f( mesh.vertex[ mesh.polygon[l_index].b ].x,
                    mesh.vertex[ mesh.polygon[l_index].b ].y,
                    mesh.vertex[ mesh.polygon[l_index].b ].z);
        
        //----------------- THIRD VERTEX -----------------
        // Texture coordinates of the third vertex
        glTexCoord2f( mesh.mapcoord[ mesh.polygon[l_index].c ].u,
                      mesh.mapcoord[ mesh.polygon[l_index].c ].v);
        // Coordinates of the Third vertex
        glVertex3f( mesh.vertex[ mesh.polygon[l_index].c ].x,
                    mesh.vertex[ mesh.polygon[l_index].c ].y,
                    mesh.vertex[ mesh.polygon[l_index].c ].z);
    }
    glEnd();

    glFlush();
    glutSwapBuffers();
}

int main(int argc, char **argv) {

    int mainMenu, textureMenu, shaderMenu, materialMenu, lightMenu;
    glutInit(&argc, argv);    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(screen_width,screen_height);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("OpenGL 3ds Mesh Loading Example");    
    glutDisplayFunc(display);
    glutIdleFunc(display);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);
    glutSpecialFunc(keyboard_s);

#define TEXTURE_ENTRIES() \
    glutAddMenuEntry("Texture 1", TEXTURE1); \
    glutAddMenuEntry("Texture 2", TEXTURE2);  \
    glutAddMenuEntry("Texture 3", TEXTURE3);  \
    glutAddMenuEntry("Texture 4", TEXTURE4);  \
    glutAddMenuEntry("Texture 5", TEXTURE5);  \

#define SHADER_ENTRIES() \
    glutAddMenuEntry("Normal Shading", NORMAL); \
    glutAddMenuEntry("Per-pixel lighting", PERPIXEL); \
    glutAddMenuEntry("Phong Shading", PHONG); \
    glutAddMenuEntry("Toon Shading", TOON); \
    glutAddMenuEntry("Glass", GLASS); \

#define MATERIAL_ENTRIES() \
    glutAddMenuEntry("Red", R); \
    glutAddMenuEntry("Green", G); \
    glutAddMenuEntry("Blue", B); \

    

#define LIGHT_ENTRIES() \
    glutAddMenuEntry("On", ON); \
    glutAddMenuEntry("Off", OFF); \
    

    
    materialMenu = glutCreateMenu(materialMenuHandler);
    MATERIAL_ENTRIES();


    lightMenu = glutCreateMenu(lightMenuHandler);
    LIGHT_ENTRIES();

    textureMenu = glutCreateMenu(textureMenuHandler);
    TEXTURE_ENTRIES();

    shaderMenu = glutCreateMenu(shaderMenuHandler);
    SHADER_ENTRIES();


    mainMenu = glutCreateMenu(mainMenuHandler);
    glutAddSubMenu("Lights", lightMenu);
    glutAddSubMenu("Material", materialMenu);
    glutAddSubMenu("Shaders", shaderMenu);
    glutAddSubMenu("Textures", textureMenu);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glLightfv(GL_LIGHT0, GL_POSITION, light1_position);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT1, GL_POSITION, light2_position);
    glLightfv(GL_LIGHT1, GL_SPECULAR, white_light);
    glLightfv(GL_LIGHT2, GL_POSITION, light3_position);
    glLightfv(GL_LIGHT2, GL_SPECULAR, white_light);

    glEnable(GL_LIGHTING);

    init();
    glewInit();
    if (glewIsSupported("GL_VERSION_2_0"))
        printf("Ready for OpenGL 2.0\n");
    else {
        printf("OpenGL 2.0 not supported\n");
        exit(1);
    }
    setShaders();
    glutMainLoop();
    return(0);    
}
