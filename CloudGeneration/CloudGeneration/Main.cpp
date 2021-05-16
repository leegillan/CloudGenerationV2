#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <vector>

#include "Scene.h"

Scene* scene;
Input* input;

int oldTimeSinceStart = 0;

void Init()
{
    //glewInit();

    //Initialize glew so that new OpenGL function names can be used
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error! w/ GLEW." << std::endl;
    }

    input = new Input();

    //initialise scene
    scene = new Scene();
    scene->Init(input);
}

// scene object to refresh the OpenGL buffers to the new dimensions.
void ChangeSize(int w, int h)
{
    scene->Resize(w, h);
}

//Handles keyboard input events from GLUT.
// Called whenever a "normal" key is released.
void processNormalKeys(unsigned char key, int x, int y)
{
    // If the ESCAPE key was pressed, exit application.
    if (key == VK_ESCAPE)
    {
        exit(0);
    }

    // Send key down to input class.
    input->SetKeyDown(key);
}

void processNormalKeysUp(unsigned char key, int x, int y)
{
    // Send key up to input class.
    input->SetKeyUp(key);
}

// Calculates delta time (the time between frames, in seconds)
// Calls Scene update and render for the next frame.
void RenderScene(void)
{
    // Calculate delta time.
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);

    float deltaTime = (float)timeSinceStart - (float)oldTimeSinceStart;

    oldTimeSinceStart = timeSinceStart;
    deltaTime = deltaTime / 100.0f;

    // Update Scene and render next frame.
   // scene->HandleInput(deltaTime);
    scene->Update(deltaTime, (float)timeSinceStart);
}

//main program loop
int main(int argc, char** argv)
{
    //setup OpenGL
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_STENCIL);

    glutInitWindowPosition(480, 270);                // distance from the top-left screen
    glutInitWindowSize(960, 540);                    // window size

    int win = glutCreateWindow("CMP400 - Project");  // message displayed on top bar window

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Register callback functions for change in size and rendering.
    glutDisplayFunc(RenderScene);
    glutReshapeFunc(ChangeSize);

    glutKeyboardFunc(processNormalKeys);
    glutKeyboardUpFunc(processNormalKeysUp);

    glutIdleFunc(RenderScene);

    //initialise program requirements
    Init();

    //Position mouse in centre of windows before main loop (window not resized yet)
    glutWarpPointer(400, 300);

    // Enter GLUT event processing cycle
    glutMainLoop();

	return 0;
}