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
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
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
	glBlendEquation(GL_COMBINE);

	/*glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.1f);*/
}

void Scene::Init()
{
	//camera setup 
	camera = new Camera();
	camera->SetProjectionMatrix(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), nearPlane, farPlane);
	//put y to 1 to make it look like its above camera
	camera->SetCameraPos(glm::vec3(0.0f, 0.0f, 9.f));

	//setup planes
	for (int i = 0; i < numOfPlanes; i++)
	{
		VertexArrayObject object = InitPlane();
		planeObjects.push_back(object);

		randPosX[i] = randNum(-2.f, 2.f);
		randPosY[i] = randNum(-2.f, 2.f);

		planeObjects[i].vA->Unbind();
		planeObjects[i].vB->Unbind();
		planeObjects[i].vB->Unbind();
		planeObjects[i].iB->Unbind();
	}

	for (int i = 0; i < numOfSpheres; i++)
	{
		VertexArrayObject object = InitSphere(10.0f, 40.0f, 40.0f);
		sphereObjects.push_back(object);

		randSpherePosX[i] = randNum(-1.f, 1.f);
		randSpherePosY[i] = randNum(-1.f, 1.f);

		sphereObjects[i].vA->Unbind();
		sphereObjects[i].vB->Unbind();
		sphereObjects[i].vB->Unbind();
		sphereObjects[i].iB->Unbind();
	}

	//shader setup
	basicShader = new Shader("Shaders/Shader.shader");
	basicShader->Bind();

	texture = new Texture();
	texture->GenerateTexture2D(128, 128);

	//texture setup for passing through shader
	for (int i = 0; i < 2; i++)
	{
		texture->Bind(i);

		basicShader->SetUniform1i("uTexture[" + std::to_string(i) + "]", i);

		texture->Unbind();
	}

	//pass texture to shader
	basicShader->SetUniform2f("uResolution", glm::vec2(GLUT_WINDOW_WIDTH, GLUT_WINDOW_HEIGHT));

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

	for (int i = 0; i < 25; i++)
	{
		std::cout << nucleation[i] << std::endl;
		std::cout << std::log(nucleation[i]) << std::endl;
	}

	float deltaN = nucleation[24] - nucleation[1];

	for (int j = 0; j < 24; j++)
	{
		nucleationRange[j] = (nucleation[j + 1] / deltaN);
	
		std::cout << nucleationRange[j] << std::endl;
	}
}

//update scene
void Scene::Update(float dt, float timePassed)
{
	// Calculate FPS for output
	CalculateFPS();

	//update camera
	camera->Update(dt);

	Render(dt, timePassed);
}

//render scene
void Scene::Render(float dt, float timePassed)
{
	// Clear Color and Depth Buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Reset transformations
	glLoadIdentity();

	camera->SetCameraLook(glm::vec3(camera->GetForward().x, camera->GetForward().y, camera->GetForward().z));

	//camera->SetCameraLook(glm::vec3(camera->GetForward().x, -4.0, camera->GetForward().z));

	//set the new view matrix for updating
	camera->SetViewMatrix();

	//glPolygonMode(GL_FRONT, GL_LINE);

	Draw(timePassed);

	basicShader->Unbind();

	// Render text, should be last object rendered.
	RenderTextOutput();

	// Swap buffers, after all objects are rendered.
	glutSwapBuffers();
}

void Scene::Draw(float timePassed)
{
	basicShader->Bind(); //Bind shader

	basicShader->SetUniformMat4("viewMatrix", camera->GetViewMatrix());
	basicShader->SetUniformMat4("projectionMatrix", camera->GetProjectionMatrix());

	basicShader->SetUniform1f("curNucVal", 0.01);
	basicShader->SetUniform1f("nextNucVal", 0.01);

	//Planes 2D
	float height = -0.5f;
	float altitudeCheck = 0.0f;

	int textureID = 0;

	{
		//for (int j = 0; j < 10; j++)
		//{
		//	for (int k = 0; k < 10; k++)
		//	{
		//		//Pass time passed to shader
		//		basicShader->SetUniform1f("uTime", timePassed);

		//		texture->Bind(0);

		//		basicShader->SetUniform1i("uTexture" + std::to_string(textureCount), textureCount);
		//		basicShader->SetUniform1i("usingTex1", tex1);

		//		basicShader->SetUniform1f("curNucVal", 0.02);
		//		basicShader->SetUniform1f("nextNucVal", 0.02);

		//		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(j - 4.5, k - 4.5, 2.0f));

		//		basicShader->SetUniformMat4("modelMatrix", modelMatrix);

		//		planeObjects[j+k].vA->Bind(); //Bind vertex buffer
		//		planeObjects[j + k].iB->Bind(); //Bind index buffer

		//		glPushMatrix();
		//		glDrawElements(GL_TRIANGLES, planeObjects[j+k].iB->GetCount(), GL_UNSIGNED_INT, nullptr);
		//		glPopMatrix();

		//		planeObjects[j + k].vA->Unbind(); //Unbind vertex buffer
		//		planeObjects[j + k].iB->Unbind(); //Unbind index buffer
		//	}
		//}
	}

	for (int i = 0; i < numOfPlanes; i++)
	{
		//Pass time passed to shader
		basicShader->SetUniform1f("uTime", timePassed);

		//Texture Stuff
		{
			//Texture Update
			if (i % 2 == 0.)
			{
				textureID = 0;
				texture->Bind(0);
			}
			else
			{
				textureID = 1;
				texture->Bind(1);
			}

			//basicShader->SetUniform1i("uTexture" + std::to_string(textureCount), textureCount);
			basicShader->SetUniform1i("textureID", textureID);

			if ((i % (numOfPlanes / nucleation.size())) == 0.0)
			{
				altitudeCheck = nucleationRange[nucleation.size() - (i / (numOfPlanes / nucleation.size()))];

				if (basicShader->isBound == true)
				{
					basicShader->SetUniform1f("curNucVal", altitudeCheck);
					basicShader->SetUniform1f("nextNucVal", nucleationRange[(nucleation.size() - (i / (numOfPlanes / nucleation.size()))) + 1]);
				}
			}
		}

		//Object Stuff
		{
			height += 0.005f; //randNum(0.0f, 0.0005f);

			//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, height));
			//glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i - 4.5, 0.0, 0.0f));

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

VertexArrayObject Scene::InitSphere(double r, int lats, int longs)
{
	VertexArrayObject object;

	int i, j;

	std::vector<float> vertices;
	std::vector<unsigned int> indices;

	int indicator = 0;

	for (i = 0; i <= lats; i++)
	{
		double lat0 = 3.14159 * (-0.5 + (double)(i - 1) / lats);
		double z0 = sin(lat0);
		double zr0 = cos(lat0);

		double lat1 = 3.14159 * (-0.5 + (double)i / lats);
		double z1 = sin(lat1);
		double zr1 = cos(lat1);

		for (j = 0; j <= longs; j++)
		{
			double lng = 2 * 3.14159 * (double)(j - 1) / longs;
			double x = cos(lng);
			double y = sin(lng);

			glNormal3f(x * zr0, y * zr0, z0);
			glVertex3f(r * x * zr0, r * y * zr0, r * z0);
			glNormal3f(x * zr1, y * zr1, z1);
			glVertex3f(r * x * zr1, r * y * zr1, r * z1);

			vertices.push_back(x * zr0);
			vertices.push_back(y * zr0);
			vertices.push_back(z0);

			vertices.push_back(0.0f);
			vertices.push_back(0.0f);

			indices.push_back(indicator);
			indicator++;

			vertices.push_back(x * zr1);
			vertices.push_back(y * zr1);
			vertices.push_back(z1);

			vertices.push_back(1.0f);
			vertices.push_back(1.0f);

			indices.push_back(indicator);
			indicator++;
		}

		indices.push_back(GL_PRIMITIVE_RESTART_FIXED_INDEX);
	}

	//create vertex buffers and vertex coords
	object.vA = new VertexArray();
	object.vB = new VertexBuffer(vertices.data(), vertices.size() * sizeof(float));

	//pointer to layout 0
	object.vAB.Push<float>(3);	//vertices
	object.vAB.Push<float>(2);	//tex Coords

	object.vA->AddBuffer(*object.vB, object.vAB);

	object.iB = new IndexBuffer(indices.data(), indices.size());

	return object;
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

	DisplayText(-1.f, 0.96f, 1.f, 0.f, 0.f, mouseText);

	DisplayText(-1.f, 0.90f, 1.f, 0.f, 0.f, fps);

	DisplayText(-1.f, 0.84f, 1.f, 0.f, 0.f, cameraPosText);
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