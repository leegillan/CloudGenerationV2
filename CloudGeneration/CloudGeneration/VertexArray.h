#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <vector>

#include "VertexBuffer.h"

struct VertexBufferLayout
{
	unsigned int type;
	unsigned int count;
	unsigned char normalised;

	static unsigned int GetSizeoftype(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:			return 4;
		case GL_UNSIGNED_INT:	return 4;
		case GL_UNSIGNED_BYTE:	return 1;
		}

		return 0;
	}
};

class VertexArrayBuffer
{
public:
	VertexArrayBuffer();

	template<typename T>
	void Push(unsigned int count) { static_assert(false); }

	template<>
	void Push<float>(unsigned int count)
	{
		vBLayouts.push_back({ GL_FLOAT, count, GL_FALSE });
		stride += count * VertexBufferLayout::GetSizeoftype(GL_FLOAT);
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		vBLayouts.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		stride += count * VertexBufferLayout::GetSizeoftype(GL_UNSIGNED_INT);
	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		vBLayouts.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		stride += count * VertexBufferLayout::GetSizeoftype(GL_UNSIGNED_BYTE);
	}

	const std::vector<VertexBufferLayout>& GetVertexLayouts() { return vBLayouts; }
	unsigned int GetStride() { return stride; }

private:
	unsigned int stride;

	std::vector<VertexBufferLayout> vBLayouts;
};

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	void AddBuffer(VertexBuffer& vB, VertexArrayBuffer& layout);

	void Bind();
	void Unbind();

private:
	unsigned int rendererID;
};

