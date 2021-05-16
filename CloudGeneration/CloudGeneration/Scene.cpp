#include "Scene.h"
#include <iostream>
#include <fstream>
#include <string>	
#include <sstream>
#include <random>

Scene::Scene()
{
	//OpenGL settings
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.3f, 0.8f, 1.0f);				// blue Background
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glClearStencil(0);									// Clear stencil buffer
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_TEXTURE_3D);
	glEnable(GL_MULTISAMPLE);

	//Blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	glBlendEquation(GL_COMBINE);

	/*glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);*/
}

void Scene::Init(Input* in)
{
	input = in;

	//camera setup 
	camera = new Camera();
	camera->SetProjectionMatrix(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), nearPlane, farPlane);
	//put y to 1 to make it look like its above camera
	camera->SetCameraPos(glm::vec3(0.0f, 2.0f, 4.f));

	boundaryX = 1;
	boundaryY = 1;

	//setup planes
	for (int i = 0; i < numOfPlanes; i++)
	{
		VertexArrayObject object = InitPlane();
		planeObjects.push_back(object);

		randPosX[i] = randNum(-boundaryX, boundaryX);
		randPosY[i] = randNum(-boundaryY, boundaryY);

		planeObjects[i].vA->Unbind();
		planeObjects[i].vB->Unbind();
		planeObjects[i].vB->Unbind();
		planeObjects[i].iB->Unbind();
	}

	//shader setup
	basicShader = new Shader("Shaders/Shader.shader");
	basicShader->Bind();

	texture = new Texture();
	texture->GenerateTexture2D(128, 128);

	//texture setup for passing through shader
	for (int i = 0; i < 5; i++)
	{
		texture->Bind(i);

		basicShader->SetUniform1i("uTexture[" + std::to_string(i) + "]", i);

		texture->Unbind();
	}

	//pass texture to shader
	basicShader->SetUniform2f("uResolution", glm::vec2(GLUT_WINDOW_WIDTH, GLUT_WINDOW_HEIGHT));

	backgroundColour = glm::vec4(0.0, 0.3f, 0.8f, 0.2);
	basicShader->SetUniform4f("uColour", backgroundColour);

	//texture setup for passing through shader
	for (int i = 0; i < 10; i++)
	{
		randNumber[i] = randNum(0.f, 1.f);
		basicShader->SetUniform1f("randNumber0to1[" + std::to_string(i) + "]", randNumber[i]);
	}

	basicShader->Unbind();

	//initialise nulceation calcs
	nuc = new Nucleation();
	nuc->CalcAtmosphere();

	//Nucleation results tests
	nucleation = nuc->GetNucleation();

	std::cout << "This is the end. Onto normal nuc values" << std::endl;

	for (int i = 0; i < 25; i++)
	{
		std::cout << nucleation[i] << std::endl;
	}

	std::cout << "This is the end. Onto normal range" << std::endl;

	float deltaN = nucleation[24] - nucleation[1];

	std::cout << deltaN << std::endl;

	for (int j = 0; j < 24; j++)
	{
		nucleationRange[j] = (nucleation[j + 1] / deltaN);

		std::cout << nucleationRange[j] << std::endl;
	}

	//you end up with values ranging from 0 to 1 but with large bias towards 0^28

	std::cout << "This is the end. Onto normal log nuc values" << std::endl;

	for (int i = 0; i < 25; i++)
	{
		std::cout << std::log(nucleation[i]) / 100 << std::endl;
	}

	std::cout << "This is the end. Onto log range" << std::endl;

	float logDeltaN = log(nucleation[24]) - log(nucleation[2]);

	std::cout << logDeltaN << std::endl;

	float logNucleationRange[25];

	for (int j = 0; j < 24; j++)
	{
		logNucleationRange[j] = (log(nucleation[j + 1]) / logDeltaN);

		std::cout << log(logNucleationRange[j]) << std::endl;
	}

	heightDifference = 0.001f;

	passTime = true;
}

//update scene
void Scene::Update(float dt, float timePassed)
{
	// Calculate FPS for output
	CalculateFPS();

	HandleInput();

	//update camera
	camera->Update(dt);

	Render(dt, timePassed);
}

void Scene::HandleInput()
{
	//Camera positions and look

	//looking up at clouds from below
	if (input->isKeyDown('1'))
	{
		camera->SetCameraPos(glm::vec3(0.0f, 0.0f, 5.f));
		camera->SetCameraLook(glm::vec3(0.0, 0.0, 0.0));
	}

	//looking up at clouds from below further back
	if (input->isKeyDown('2'))
	{
		camera->SetCameraPos(glm::vec3(0.0f, 0.0f, 8.f));
		camera->SetCameraLook(glm::vec3(0.0, 0.0, 0.0));
	}

	//looking up at clouds from below at an angle
	if (input->isKeyDown('3'))
	{
		camera->SetCameraPos(glm::vec3(0.0f, 0.0f, 4.f));
		camera->SetCameraLook(glm::vec3(0.0, -0.5, 0.0));
	}

	if (input->isKeyDown('q'))
	{
		camera->SetCameraLook(glm::vec3(camera->GetCameraLook().x , camera->GetCameraLook().y, camera->GetCameraLook().z + 0.1));
	}

	if (input->isKeyDown('z'))
	{
		camera->SetCameraLook(glm::vec3(camera->GetCameraLook().x, camera->GetCameraLook().y , camera->GetCameraLook().z - 0.1));
	}

	if (input->isKeyDown('w'))
	{
		camera->SetCameraPos(glm::vec3(camera->GetCameraPos().x, camera->GetCameraPos().y, camera->GetCameraPos().z - 0.1));
	}

	if (input->isKeyDown('s'))
	{
		camera->SetCameraPos(glm::vec3(camera->GetCameraPos().x, camera->GetCameraPos().y, camera->GetCameraPos().z + 0.1));
	}

	if (input->isKeyDown('a'))
	{
		camera->SetCameraPos(glm::vec3(camera->GetCameraPos().x - 0.1, camera->GetCameraPos().y, camera->GetCameraPos().z));
	}

	if (input->isKeyDown('d'))
	{
		camera->SetCameraPos(glm::vec3(camera->GetCameraPos().x + 0.1, camera->GetCameraPos().y, camera->GetCameraPos().z ));
	}

	if (input->isKeyDown(';'))
	{
		camera->SetCameraPos(glm::vec3(camera->GetCameraPos().x , camera->GetCameraPos().y + 0.1, camera->GetCameraPos().z));
		camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));
	}

	if (input->isKeyDown('p'))
	{
		camera->SetCameraPos(glm::vec3(camera->GetCameraPos().x, camera->GetCameraPos().y - 0.1, camera->GetCameraPos().z));

		camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));
	}

	if (input->isKeyDown('4'))
	{
		camera->SetCameraPos(glm::vec3(0.0f, 5.0f, 0.f));
		camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));
	}

	if (input->isKeyDown('5'))
	{
		camera->SetCameraPos(glm::vec3(0.0f, 5.0f, 3.f));
		camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));
	}

	if (input->isKeyDown('6'))
	{
		camera->SetCameraPos(glm::vec3(0.0f, 4.0f, 2.f));
		camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));
	}

	if (input->isKeyDown('7'))
	{
		camera->SetCameraPos(glm::vec3(0.0f, 3.0f, 3.f));
		camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));
	}

	if (input->isKeyDown('b'))
	{
		if (bgToggle == 1)
		{
			glClearColor(0.0f, 0.3f, 0.8f, 1.0f);	// blue Background
			backgroundColour = glm::vec4(0., 0.3f, 0.8f, 0.2);
			bgToggle = 0;
		}
		else if (bgToggle == 0)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);	// black Background
			backgroundColour = glm::vec4(0., 0.0f, 0.0f, 0.2);
			bgToggle = 1;
		}
		
		input->SetKeyUp('b');
	}

	if(input->isKeyDown('r'))
	{
		RandomiseCloudLocations();	
	}
	
	ChangeCloudBoundaries();

	if (input->isKeyDown('h'))
	{
		heightDifference += 0.0001f;
	}

	if (input->isKeyDown('n'))
	{
		heightDifference -= 0.0001f;
	}

	if (input->isKeyDown('t'))
	{
		passTime = !passTime;
		input->SetKeyUp('t');
	}

}

void Scene::RandomiseCloudLocations()
{
	for (int i = 0; i < numOfPlanes; i++)
	{
		randPosX[i] = randNum(-boundaryX, boundaryX);
		randPosY[i] = randNum(-boundaryY, boundaryY);
	}
}

void Scene::ChangeCloudBoundaries()
{
	if (input->isKeyDown('i'))
	{
		boundaryX += 0.01f;
		RandomiseCloudLocations();
	}

	if (input->isKeyDown('o') && boundaryX >= 0.01f)
	{
		boundaryX -= 0.01f;
		RandomiseCloudLocations();
	}

	if (input->isKeyDown('-'))
	{
		boundaryY += 0.01f;
		RandomiseCloudLocations();
	}

	if (input->isKeyDown('=') && boundaryY >= 0.01f)
	{
		boundaryY -= 0.01f;
		RandomiseCloudLocations();
	}
}

//render scene
void Scene::Render(float dt, float timePassed)
{
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();

	//camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));
	//camera->SetCameraLook(glm::vec3(camera->GetForward().x, -4.0, camera->GetForward().z));

	camera->SetCameraLook(camera->GetCameraLook());

	//set the new view matrix for updating
	camera->SetViewMatrix();

	//glPolygonMode(GL_FRONT, GL_LINE);

	basicShader->Bind(); //Bind shader

	Draw(timePassed);

	basicShader->Unbind();

	// Render text, should be last object rendered.
	RenderTextOutput();

	// Swap buffers, after all objects are rendered.
	glutSwapBuffers();
}

void Scene::Draw(float timePassed)
{
	basicShader->SetUniformMat4("viewMatrix", camera->GetViewMatrix());
	basicShader->SetUniformMat4("projectionMatrix", camera->GetProjectionMatrix());

	basicShader->SetUniform1f("curNucVal", 0.01);
	basicShader->SetUniform1f("nextNucVal", 0.01);
	basicShader->SetUniform4f("uColour", backgroundColour);

	if (passTime == false)
	{
		//Pass time passed to shader
		basicShader->SetUniform1f("uVTime", timePassed);
		basicShader->SetUniform1f("uFTime", timePassed);
	}

	float height = -0.5f;
	int altitude = 0;
	float altitudeCheck = 0.0f;
	int textureID = 0;

	for (int i = 0; i < numOfPlanes; i++)
	{
		if (passTime == true)
		{
			//Pass time passed to shader
			basicShader->SetUniform1f("uVTime", timePassed);
			basicShader->SetUniform1f("uFTime", timePassed);
		}

		///// Texture /////
		//Texture Update
		if (i % 1 == 0.)
		{
			textureID = 0;
			texture->Bind(textureID);
		}
		if (i % 2 == 0.)
		{
			textureID = 1;
			texture->Bind(textureID);
		}
		if (i % 3 == 0.)
		{
			textureID = 2;
			texture->Bind(textureID);
		}
		if (i % 4 == 0.)
		{
			textureID = 3;
			texture->Bind(textureID);
		}
		if (i % 5 == 0.)
		{
			textureID = 4;
			texture->Bind(textureID);
		}

		basicShader->SetUniform1i("textureID", textureID);
		
		///// Altitude /////
		if ((i % (numOfPlanes / nucleation.size())) == 0.0)
		{
			altitude++;
			basicShader->SetUniform1f("uAltitude", altitude);

			altitudeCheck = nucleationRange[nucleation.size() - (i / (numOfPlanes / nucleation.size()))];
			basicShader->SetUniform1f("curNucVal", altitudeCheck);
			basicShader->SetUniform1f("nextNucVal", nucleationRange[(nucleation.size() - (i / (numOfPlanes / nucleation.size()))) + 1]);
		}
		
		//Object Stuff
		{
			height += heightDifference; //randNum(0.0f, 0.0005f);

			//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, height));
			//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i - 4.5, 0., 0.));

			glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(randPosX[i], randPosY[i], height));
			glm::mat4 planeRotateMat = glm::rotate(modelMatrix, (0.0f * glm::pi<float>() / 180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			modelMatrix *= planeRotateMat;

			basicShader->SetUniformMat4("modelMatrix", modelMatrix);

			planeObjects[i].vA->Bind(); //Bind vertex buffer
			planeObjects[i].iB->Bind(); //Bind index buffer

			glPushMatrix();
				glDrawElements(GL_TRIANGLES, planeObjects[i].iB->GetCount(), GL_UNSIGNED_INT, nullptr);
			glPopMatrix();

			planeObjects[i].vA->Unbind(); //Unbind vertex buffer
			planeObjects[i].iB->Unbind(); //Unbind index buffer
		}

		texture->Unbind();
	}
}

//plane initialisation
VertexArrayObject Scene::InitPlane()
{
	VertexArrayObject object;

	float squarePositions[] =			//final two coordinates = texCoords
	{
		-1.0f, -1.0f, 0.0f,	0.0f, 0.0f,	//0
		 1.0f, -1.0f, 0.0f,	1.0f, 0.0f, //1
		 1.0f,  1.0f, 0.0f,	1.0f, 1.0f, //2
		-1.0f,	1.0f, 0.0f,	0.0f, 1.0f, //3 
	};

	//create vertex buffers and vertex coords
	object.vA = new VertexArray();
	object.vB = new VertexBuffer(squarePositions, 4 * 5 * sizeof(float));

	//pointer to layout 0
	object.vAB.Push<float>(3);	//vertices/position layout 0

	//pointer to layout 1
	object.vAB.Push<float>(2);	//texCoords layout 1

	object.vA->AddBuffer(*object.vB, object.vAB);

	unsigned int squareIndices[] =
	{
		0, 1, 2,
		2, 3, 0
	};

	object.iB = new IndexBuffer(squareIndices, 6);

	return object;
}

double Scene::randNum(double low, double high)
{
	double rand;

	std::random_device rd; // obtain a random number from hardware
	std::mt19937 gen(rd()); // seed the generator
	std::uniform_real_distribution<> distr(low, high); // define the range

	rand = distr(gen);

	return rand;
}

////////////////////////	OpenGL functions	////////////////////////

// Handles the resize of the window. If the window changes size the perspective matrix requires re-calculation to match new window size.
void Scene::Resize(int w, int h)
{
	width = w;
	height = h;

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	float ratio = (float)w / (float)h;
	fov = 60.0f;
	nearPlane = 0.01f;
	farPlane = 100.0f;

	// Use the Projection Matrix
	glMatrixMode(GL_PROJECTION);

	// Reset Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the correct perspective.
	gluPerspective(fov, ratio, nearPlane, farPlane);
	camera->SetProjectionMatrix(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), nearPlane, farPlane);

	// Get Back to the Modelview
	glMatrixMode(GL_MODELVIEW);
}

// Calculates FPS
void Scene::CalculateFPS()
{
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);

	if (time - timebase > 1000)
	{
		sprintf_s(fps, "FPS: %4.2f", frame * 1000.0 / (time - timebase));
		timebase = time;
		frame = 0;
	}
}

// Compiles standard output text including FPS and current mouse position.
void Scene::RenderTextOutput()
{
	// Render current mouse position and frames per second.
	//sprintf_s(mouseText, "Mouse: %i, %i", input->getMouseX(), input->getMouseY());

	sprintf_s(cameraPosText, "Cam Position: %.2f, %.2f, %.2f", camera->GetCameraPos().x, camera->GetCameraPos().y, camera->GetCameraPos().z);

	sprintf_s(cloudBoundaries, "Cloud Boundaries: X: %.2f and Y: %.2f", boundaryX, boundaryY);

	sprintf_s(passTimeText, "Passing Time per object draw: %.1i", (int)passTime);

	DisplayText(-1.f, 0.96f, 1.f, 0.f, 0.f, mouseText);

	DisplayText(-1.f, 0.90f, 1.f, 0.f, 0.f, fps);

	DisplayText(-1.f, 0.84f, 1.f, 0.f, 0.f, cameraPosText);

	DisplayText(-1.f, 0.78f, 1.f, 0.f, 0.f, cloudBoundaries);

	DisplayText(-1.f, 0.72f, 1.f, 0.f, 0.f, passTimeText);
}

// Renders text to screen. Must be called last in render function (before swap buffers)
void Scene::DisplayText(float x, float y, float r, float g, float b, char* string) 
{
	// Get Lenth of string
	int j = strlen(string);

	// Swap to 2D rendering
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 5, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Orthographic lookAt (along the z-axis).
	gluLookAt(0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Set text colour and position.
	glColor3f(r, g, b);
	glRasterPos2f(x, y);

	// Render text.
	for (int i = 0; i < j; i++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, string[i]);
	}

	// Reset colour to white.
	glColor3f(1.f, 1.f, 1.f);

	// Swap back to 3D rendering.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, ((float)width / (float)height), nearPlane, farPlane);
	glMatrixMode(GL_MODELVIEW);
}