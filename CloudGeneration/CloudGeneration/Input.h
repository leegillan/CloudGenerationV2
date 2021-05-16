#pragma once

#include "Camera.h"

class Input
{
	struct Mouse
	{
		int x, y;
		bool left, right;
	};

public:
	void Update(float dt, Camera* camera);

	//Getters
	int getMouseX();
	int getMouseY();
	bool isLeftMouseButtonPressed();

	bool isKeyDown(int);

	//Setters
	void setMouseX(int);
	void setMouseY(int);
	void setMousePos(int x, int y);
	void setLeftMouseButton(bool b);

	void SetKeyDown(unsigned char key);
	void SetKeyUp(unsigned char key);

	// Boolean array, element per key
	bool keys[256];

	Mouse mouse;
};

