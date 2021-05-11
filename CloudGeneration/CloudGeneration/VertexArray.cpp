#include "VertexArray.h"

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &rendererID);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &rendererID);
}

void VertexArray::AddBuffer(VertexBuffer& vB, VertexArrayBuffer& layout)
{
	Bind();
	vB.Bind();

	const auto& elements = layout.GetVertexLayouts();
	unsigned int offset = 0;

	//setup layout for vertex buffer
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& elementLayout = elements[i];

		//vertex buffer layout
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, elementLayout.count, elementLayout.type, elementLayout.normalised, layout.GetStride(), (const void*)offset);	//links buffer with vertexArrayObject
		//offset += elementLayout.count * elementLayout.GetSizeoftype(elementLayout.type);
		offset += elementLayout.count * VertexBufferLayout::GetSizeoftype(elementLayout.type);
	}
}

void VertexArray::Bind()
{
	glBindVertexArray(rendererID);
}

void VertexArray::Unbind()
{
	glBindVertexArray(0);
}

VertexArrayBuffer::VertexArrayBuffer()
{
	stride = 0;
}