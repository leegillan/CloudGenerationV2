#include "Input.h"


void Input::Update(float dt, Camera* camera)
{

}

void Input::SetKeyDown(unsigned char key)
{
	keys[key] = true;
}

void Input::SetKeyUp(unsigned char key)
{
	keys[key] = false;
}

bool Input::isKeyDown(int key)
{
	return keys[key];
}

int Input::getMouseX()
{
	return mouse.x;
}

int Input::getMouseY()
{
	return mouse.y;
}

void Input::setMouseX(int pos)
{
	mouse.x = pos;
}

void Input::setMouseY(int pos)
{
	mouse.y = pos;
}

void Input::setMousePos(int ix, int iy)
{
	mouse.x = ix;
	mouse.y = iy;
}

void Input::setLeftMouseButton(bool b)
{
	mouse.left = b;
}

bool Input::isLeftMouseButtonPressed()
{
	return mouse.left;
}