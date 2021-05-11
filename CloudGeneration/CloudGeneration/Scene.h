#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "Camera.h"
#include "Nucleation.h"

#include<vector>

struct VertexArrayObject
{
	VertexArray* vA;
	VertexArrayBuffer vAB;
	VertexBuffer* vB;
	IndexBuffer* iB;
};

class Scene
{
public:
	Scene();

	//Inititalises the class
	void Init();

	//Main render function
	void Render(float dt, float timePassed);

	//draws to screen
	void Draw(float dt);

	//Handle input function that receives delta time from parent.
	//void HandleInput(float dt);

	//Update function receives delta time from parent (used for frame independent updating).
	void Update(float dt, float timePassed);

	//Resizes the OpenGL output based on new window size.
	void Resize(int w, int h);

protected:
	// Renders text (x, y positions, RGB colour of text, string of text to be rendered)
	void DisplayText(float x, float y, float r, float g, float b, char* string);

	// A function to collate all text output in a single location
	void RenderTextOutput();

	void CalculateFPS();

	double randNum(double low, double high);

	VertexArrayObject InitPlane();
	VertexArrayObject InitSphere(double r, int lats, int longs);

	//class objects
	Shader* basicShader;
	Shader* basicShader2;
	Camera* camera;
	Texture* texture;
	Nucleation* nuc;

	std::vector<double> nucleation;
	float nucleationRange[25];

	//group of planes
	std::vector<VertexArrayObject> planeObjects;
	const static int numOfPlanes = 1000;
	double randPosX[numOfPlanes];
	double randPosY[numOfPlanes];

	//group of spheres
	std::vector<VertexArrayObject> sphereObjects;
	const static int numOfSpheres = 100;
	double randSpherePosX[numOfSpheres];
	double randSpherePosY[numOfSpheres];

	float randNumber[10];
	int randTexture;

	// For Window and frustum calculation.
	int width = 0, height = 0;
	float fov = 0, nearPlane = 0, farPlane = 0;

	// For FPS counter and mouse coordinate output.
	int frame = 0, time = 0, timebase = 0;
	char fps[40];
	char mouseText[40];
	char cameraPosText[40];

	//Shader variables
	int location = 0;
};

