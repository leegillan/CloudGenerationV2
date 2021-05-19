#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>


#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

#include "Shader.h"
#include "Texture.h"

#include "Camera.h"
#include "Input.h"
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
	void Init(Input* in);

	//Main render function
	void Render(float dt, float timePassed);

	//draws to screen
	void Draw(float dt);

	//Handle input function that receives delta time from parent.
	void HandleInput();

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

	void RandomiseCloudLocations();
	void ChangeCloudBoundaries();

	VertexArrayObject InitPlane();

	//class objects
	Shader* basicShader;
	Camera* camera;
	Input* input;
	Texture* texture;
	Nucleation* nuc;

	std::vector<double> nucleation;
	float nucleationRange[100];
	float shaderNucleation[100];

	//nucleation temp
	float temp = 293.;

	//group of planes
	std::vector<VertexArrayObject> planeObjects;
	const static int maxNumberofPlanes = 5000;
	const static int minNumberofPlanes = 100;
	double randPosX[maxNumberofPlanes];
	double randPosY[maxNumberofPlanes];
	int numOfPlanes;

	float randNumber[10];
	int randTexture;

	float boundaryX;
	float boundaryY;

	float heightDifference;

	//background toggler
	int bgToggle = 0;
	glm::vec4 backgroundColour;

	// For Window and frustum calculation.
	int width = 0, height = 0;
	float fov = 0, nearPlane = 0, farPlane = 0;

	// For FPS counter and mouse coordinate output.
	int frame = 0, time = 0, timebase = 0;
	char fps[40];
	char mouseText[40];
	char cameraPosText[40];

	int usingNuc;
	int drawInLine;

	bool passTime;
	char passTimeText[40];

	char usingNucText[40];

	//boundary text
	char cloudBoundaries[40];

	char planeCountText[40];

	//camera location text
	char cameraLoc[40];

	//Shader variables
	int location = 0;
};

