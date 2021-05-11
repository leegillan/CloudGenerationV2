#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>

#include <SOIL/SOIL.h>
#include <vector>

class Texture
{
public:
	Texture();
	~Texture();

	void LoadTexture(const std::string& filePath);
	void GenerateTexture2D(int width, int height);
	void GenerateTexture3D(int width, int height, int length);

	void Bind(unsigned int slot);
	void Unbind();

	void Bind3D(unsigned int slot);
	void Unbind3D();

	int GetWidth() { return width; }
	int GetHeight() { return height; }
	int GetDepth() { return depth; }

private:
	std::string filePath;
	unsigned int rendererID;

	unsigned char* localBuffer;

	int width, height, depth, bitsPerPix;
};

