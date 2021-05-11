#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>

class IndexBuffer
{
public:
	IndexBuffer(const unsigned int* data, unsigned int count);
	~IndexBuffer();

	void Bind();
	void Unbind();

	unsigned int GetCount() { return count; };

private:
	unsigned int rendererID;
	unsigned int count;
};

