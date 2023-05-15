//////////////////////////////////////////////////////////////////////////////////////////
//	Main.cpp
//	Bumpy per pixel attenuation
//	Downloaded from: www.paulsprojects.net
//	Created:	26th August 2002
//
//	Copyright (c) 2006, Paul Baker
//	Distributed under the New BSD Licence. (See accompanying file License.txt or copy at
//	http://www.paulsprojects.net/NewBSDLicense.txt)
//////////////////////////////////////////////////////////////////////////////////////////	
#include <windows.h>
#include <GL\gl.h>
#include <GL\glu.h>
#include <GL\glext.h>
#include <GL\wglext.h>
#include "LOG.h"
#include "WINDOW.h"
#include "extensions/ARB_multitexture_extension.h"
#include "extensions/EXT_compiled_vertex_array_extension.h"
#include "extensions/EXT_texture_edge_clamp_extension.h"
#include "extensions/NV_register_combiners_extension.h"
#include "extensions/NV_vertex_program_extension.h"
#include "FPS_COUNTER.h"
#include "TIMER.h"
#include "Maths/Maths.h"
#include "IMAGE.h"
#include "main.h"
#include "TORUS.h"
#include "Normalisation Cube Map.h"
#include "NV_vertex_program.h"

//link to libraries
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "winmm.lib")

//errorLog MUST be kept - it is used by other files
LOG errorLog;
WINDOW window;
FPS_COUNTER fpsCounter;
TIMER timer;

COLOR backgroundColor(0.0f, 0.0f, 0.0f, 0.0f);

const int numLights=4;
LIGHT lights[numLights];
bool drawLight[numLights]={1, 1, 1, 1};
bool drawRadii=false;

//attenuation textures
IMAGE atten2DImage, atten1DImage;
GLuint atten2DTexture, atten1DTexture;
IMAGE normalMapImage;
GLuint normalMapTexture;
GLuint normalisationCubeMap;

IMAGE lightImage;
GLuint lightTexture;

GLuint vertexProgram;

TORUS torus;

GLUquadricObj * sphere;	//used to draw light radii

//Set up variables
bool DemoInit()
{
	if(!window.Init("Per Pixel Attenuation", 640, 480, 32, 32, 0, WINDOWED_SCREEN))
		return 0;											//quit if not created
	
	SetUpARB_multitexture();
	SetUpEXT_compiled_vertex_array();
	SetUpEXT_texture_edge_clamp();
	SetUpNV_register_combiners();
	SetUpNV_vertex_program();
	
	if(	!EXT_compiled_vertex_array_supported || !EXT_texture_edge_clamp_supported ||
		!ARB_multitexture_supported || !NV_register_combiners_supported ||
		!NV_vertex_program_supported)
		return false;
	
	GLint maxTextureUnitsARB;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &maxTextureUnitsARB);
	if(maxTextureUnitsARB<4)
	{
		errorLog.OutputError("I require at least 4 texture units");
		return false;
	}

	//Load Textures
	//2d attenuation
	atten2DImage.Load("atten2d.tga");
	glGenTextures(1, &atten2DTexture);
	glBindTexture(GL_TEXTURE_2D, atten2DTexture);
	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA8, atten2DImage.width, atten2DImage.height,
					0, atten2DImage.format, GL_UNSIGNED_BYTE, atten2DImage.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//1d attenuation
	atten1DImage.Load("atten1d.tga");
	glGenTextures(1, &atten1DTexture);
	glBindTexture(GL_TEXTURE_1D, atten1DTexture);
	glTexImage1D(	GL_TEXTURE_1D, 0, GL_RGBA8, atten1DImage.width,
					0, atten1DImage.format, GL_UNSIGNED_BYTE, atten1DImage.data);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			
	//Normal map
	normalMapImage.Load("normal map.bmp");
	glGenTextures(1, &normalMapTexture);
	glBindTexture(GL_TEXTURE_2D, normalMapTexture);
	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA8, normalMapImage.width, normalMapImage.height,
					0, normalMapImage.format, GL_UNSIGNED_BYTE, normalMapImage.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Light Texture
	lightImage.Load("light.tga");
	glGenTextures(1, &lightTexture);
	glBindTexture(GL_TEXTURE_2D, lightTexture);
	glTexImage2D(	GL_TEXTURE_2D, 0, GL_RGBA8, lightImage.width, lightImage.height,
					0, lightImage.format, GL_UNSIGNED_BYTE, lightImage.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//Create normalisation cube map
	glGenTextures(1, &normalisationCubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, normalisationCubeMap);
	GenerateNormalisationCubeMap();
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	
	//set up lights
	lights[0].axis.Set(0.0f, 0.0f, 1.0f);
	lights[0].centre.Set(4.0f, 0.0f, 0.0f);
	lights[0].radius.Set(4.0f, 0.0f, 0.0f);
	lights[0].angleDifference=0.0f;
	lights[0].speed=120.0f;
	lights[0].r=1.0f;
	lights[0].g=0.0f;
	lights[0].b=0.0f;
	lights[0].lightRadius=6.0f;

	lights[1].axis.Set(0.0f, 1.0f, 0.0f);
	lights[1].centre.Set(0.0f, 0.0f, 0.0f);
	lights[1].radius.Set(8.0f, 0.0f, 0.0f);
	lights[1].angleDifference=60.0f;
	lights[1].speed=120.0f;
	lights[1].r=0.0f;
	lights[1].g=0.0f;
	lights[1].b=1.0f;
	lights[1].lightRadius=10.0f;

	lights[2].axis.Set(0.0f, 0.0f,-1.0f);
	lights[2].centre.Set(0.0f, 0.0f, 0.0f);
	lights[2].radius.Set(7.5f, 0.0f, 0.0f);
	lights[2].angleDifference=120.0f;
	lights[2].speed=60.0f;
	lights[2].r=1.0f;
	lights[2].g=1.0f;
	lights[2].b=0.0f;
	lights[2].lightRadius=7.0f;

	lights[3].axis.Set(1.0f, 0.0f, 0.0f);
	lights[3].centre.Set(0.0f, 0.0f, 0.0f);
	lights[3].radius.Set(0.0f, 7.0f, 0.0f);
	lights[3].angleDifference=180.0f;
	lights[3].speed=90.0f;
	lights[3].r=0.0f;
	lights[3].g=1.0f;
	lights[3].b=0.0f;
	lights[3].lightRadius=6.0f;

	//Set up sphere
	sphere=gluNewQuadric();
	gluQuadricDrawStyle(sphere, GLU_LINE);

	//reset timer for start
	timer.Reset();
	
	return true;
}

//Set up openGL
bool GLInit()
{
	//set viewport
	int height;
	if (window.height==0)
		height=1;
	else
		height=window.height;
	
	glViewport(0, 0, window.width, height);					//reset viewport

	//set up projection matrix
	glMatrixMode(GL_PROJECTION);							//select projection matrix
	glLoadIdentity();										//reset
	gluPerspective(45.0f, (GLfloat)window.width/(GLfloat)height, 1.0f, 100.0f);
	
	//load identity modelview
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//other states
	//shading
	glShadeModel(GL_SMOOTH);
	glClearColor(	backgroundColor.r,
					backgroundColor.g,
					backgroundColor.b,
					backgroundColor.a);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	//depth
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//hints
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);


	//Set up vertex arrays for torus
	glEnableClientState(GL_VERTEX_ARRAY);		
	glVertexPointer(3, GL_FLOAT, sizeof(TORUS_VERTEX), &torus.vertices[0].position);
	glEnableClientState(GL_NORMAL_ARRAY);
	glNormalPointer(GL_FLOAT, sizeof(TORUS_VERTEX), &torus.vertices[0].normal);

	//Send Texture Coordinates to unit 0
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(TORUS_VERTEX), &torus.vertices[0].s);
	
	//Send Tangents to unit 1,2
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(3, GL_FLOAT, sizeof(TORUS_VERTEX), &torus.vertices[0].sTangent);
	
	glActiveTextureARB(GL_TEXTURE2_ARB);
	glClientActiveTextureARB(GL_TEXTURE2_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(3, GL_FLOAT, sizeof(TORUS_VERTEX), &torus.vertices[0].tTangent);

	glActiveTextureARB(GL_TEXTURE0_ARB);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	
	//Use CVA
	glLockArraysEXT(0, torus.numVertices);



	//set up the register combiners
	//two general combiners
	glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 2);

	//combiner 0 does tex0+tex1 -> spare0
	glCombinerInputNV(	GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB,
						GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glCombinerInputNV(	GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_ZERO,
						GL_UNSIGNED_INVERT_NV, GL_RGB);
	glCombinerInputNV(	GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_TEXTURE1_ARB,
						GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glCombinerInputNV(	GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_ZERO,
						GL_UNSIGNED_INVERT_NV, GL_RGB);
	glCombinerOutputNV(	GL_COMBINER0_NV, GL_RGB, GL_DISCARD_NV, GL_DISCARD_NV, GL_SPARE0_NV,
						GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);

	//combiner 1 does tex2 dot tex3 -> spare1
	glCombinerInputNV(	GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE2_ARB,
						GL_EXPAND_NORMAL_NV, GL_RGB);
	glCombinerInputNV(	GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_TEXTURE3_ARB,
						GL_EXPAND_NORMAL_NV, GL_RGB);
	glCombinerOutputNV(	GL_COMBINER1_NV, GL_RGB, GL_SPARE1_NV, GL_DISCARD_NV, GL_DISCARD_NV,
						GL_NONE, GL_NONE, GL_TRUE, GL_FALSE, GL_FALSE);



	//final combiner outputs (1-spare0)*constant color 0*spare1
	//do constant color 0*spare1 in the EF multiplier
	glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	
	//now do (1-spare0)*EF
	glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_E_TIMES_F_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
	glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);





	//load vertex program
	glGenProgramsNV(1, &vertexProgram);
	glBindProgramNV(GL_VERTEX_PROGRAM_NV, vertexProgram);
	LoadNV_vertex_program("vertex program.txt", vertexProgram);
	
	//track matrices
	glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);
	glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_TEXTURE0_ARB, GL_IDENTITY_NV);

	return true;
}

//Perform per frame updates
void UpdateFrame()
{
	window.Update();

	float angle=(float)timer.GetTime()/800;

	//position lights
	for(int i=0; i<numLights; i++)
		lights[i].position=lights[i].centre+
							(lights[i].radius.GetRotatedAxis(
								(angle+lights[i].angleDifference)*lights[i].speed, lights[i].axis));

	//toggle drawing of lights
	if(window.isKeyPressed('1'))
	{
		drawLight[1]=!drawLight[1];
		window.SetKeyReleased('1');
	}
	if(window.isKeyPressed('2'))
	{
		drawLight[2]=!drawLight[2];
		window.SetKeyReleased('2');
	}
	if(window.isKeyPressed('3'))
	{
		drawLight[3]=!drawLight[3];
		window.SetKeyReleased('3');
	}

	//toggle drawing of radii
	if(window.isKeyPressed(' '))
	{
		drawRadii=!drawRadii;
		window.SetKeyReleased(' ');
	}
}

//draw a frame
void RenderFrame()
{
	//Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();										//reset modelview matrix

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	gluLookAt(0.0f, 10.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	//enable register combiners
	glEnable(GL_REGISTER_COMBINERS_NV);
	glEnable(GL_VERTEX_PROGRAM_NV);
	//bind vertex program
	glBindProgramNV(GL_VERTEX_PROGRAM_NV, vertexProgram);


	//draw lit
	glBlendFunc(GL_ONE, GL_ONE);

	for(int i=0; i<numLights; i++)
	{
		if(i>0)
			glEnable(GL_BLEND);

		if(!drawLight[i])
			continue;

		//send the lights color to constant color 0
		COLOR color(lights[i].r, lights[i].g, lights[i].b, 0.0f);
		glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, color);
		
		//send the light's position to vertex program parameter 8
		glProgramParameter4fNV(	GL_VERTEX_PROGRAM_NV, 8,
								lights[i].position.x, lights[i].position.y,
								lights[i].position.z, 1.0f);
						
		//set the texture matrix
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glTranslatef(0.5f, 0.5f, 0.5f);
		glScalef(0.5f, 0.5f, 0.5f);
		glScalef(1/lights[i].lightRadius, 1/lights[i].lightRadius, 1/lights[i].lightRadius);
		glTranslatef(-lights[i].position.x, -lights[i].position.y, -lights[i].position.z);
		glMatrixMode(GL_MODELVIEW);
	
		//unit 0
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, atten2DTexture);

		//unit 1
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_1D);
		glBindTexture(GL_TEXTURE_1D, atten1DTexture);

		//unit 2
		glActiveTextureARB(GL_TEXTURE2_ARB);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, normalMapTexture);

		//unit 3
		glActiveTextureARB(GL_TEXTURE3_ARB);
		glEnable(GL_TEXTURE_CUBE_MAP_ARB);
		glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, normalisationCubeMap);

		glActiveTextureARB(GL_TEXTURE0_ARB);

		//draw torus
		glDrawElements(GL_TRIANGLES, torus.numIndices, GL_UNSIGNED_INT, torus.indices);
	}
	glPopAttrib();

	//reset texture matrix
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);


	//draw light radii
	if(drawRadii)
	{
		for(int i=0; i<numLights; i++)
		{
			if(!drawLight[i])
				continue;

			glPushMatrix();
			glTranslatef(lights[i].position.x, lights[i].position.y, lights[i].position.z);
			glColor3f(lights[i].r, lights[i].g, lights[i].b);

			gluSphere(sphere, lights[i].lightRadius, 24, 12);
	
			glPopMatrix();
		}
	}



	//draw lights as billboarded quads
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	glEnable(GL_BLEND);
	
	glBindTexture(GL_TEXTURE_2D, lightTexture);
	glEnable(GL_TEXTURE_2D);

	MATRIX4X4 modelviewMatrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, modelviewMatrix);
	VECTOR3D right(	modelviewMatrix.entries[0],
					modelviewMatrix.entries[4],
					modelviewMatrix.entries[8]);
	VECTOR3D up(	modelviewMatrix.entries[1],
					modelviewMatrix.entries[5],
					modelviewMatrix.entries[9]);
	
	for(int i=0; i<numLights; i++)
	{
		if(!drawLight[i])
			continue;

		glPushMatrix();
		glTranslatef(lights[i].position.x, lights[i].position.y, lights[i].position.z);
		glColor3f(lights[i].r, lights[i].g, lights[i].b);

		float size=0.5f;

		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0f, 0.0f);
			glVertex3fv((right+up)*-size);
			glTexCoord2f(1.0f, 0.0f);
			glVertex3fv((right-up)*size);
			glTexCoord2f(1.0f, 1.0f);
			glVertex3fv((right+up)*size);
			glTexCoord2f(0.0f, 1.0f);
			glVertex3fv((right-up)*-size);
		}
		glEnd();

		glPopMatrix();
	}
	
	glPopAttrib();

	
	
	fpsCounter.Update();											//update frames per second counter
	glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
	window.StartTextMode();
	window.Print(0, 28, "FPS: %.2f", fpsCounter.GetFps());			//print the fps
	window.EndTextMode();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	if(window.isKeyPressed(VK_F1))
	{
		window.SaveScreenshot();
		window.SetKeyReleased(VK_F1);
	}

	window.SwapBuffers();									//swap buffers

	//check for any opengl errors
	window.CheckGLError();

	//quit if necessary
	if(window.isKeyPressed(VK_ESCAPE))
		PostQuitMessage(0);
}

void DemoShutdown()
{
	window.Shutdown();										//Shutdown window
}

//ENTRY POINT FOR APPLICATION
//CALL WINDOW CREATION ROUTINE, DEAL WITH MESSAGES, WATCH FOR INTERACTION
int WINAPI WinMain(	HINSTANCE	hInstance,				//instance
					HINSTANCE	hPrevInstance,			//Previous Instance
					LPSTR		lpCmdLine,				//command line parameters
					int			nCmdShow)				//Window show state
{
	//Initiation
	errorLog.Init("Error Log.txt");

	//init variables etc, then GL
	if(!DemoInit())
	{
		errorLog.OutputError("Demo Initiation failed");
		return 0;
	}
	else
		errorLog.OutputSuccess("Demo Initiation Successful");

	if(!GLInit())
	{
		errorLog.OutputError("OpenGL Initiation failed");
		return 0;
	}
	else
		errorLog.OutputSuccess("OpenGL Initiation Successful");

	//Main Loop
	for(;;)
	{
		if(!(window.HandleMessages())) break;//handle windows messages, quit if returns false
		UpdateFrame();
		RenderFrame();
	}

	DemoShutdown();
	
	errorLog.OutputSuccess("Exiting...");
	return (window.msg.wParam);								//Exit The Program
}