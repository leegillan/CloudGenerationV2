#include "Texture.h"

Texture::Texture() :
	filePath(filePath),
	rendererID(0),
	localBuffer(nullptr),
	width(0),
	height(0),
	bitsPerPix(0)
{
}

Texture::~Texture()
{
	glDeleteTextures(1, &rendererID);
}

void Texture::LoadTexture(const std::string& filePath)
{
	localBuffer = SOIL_load_image(filePath.c_str(), &width, &height, &bitsPerPix, 4);

	if (*localBuffer == NULL)
	{
		printf("[Texture loader] \"%s\" failed to load!\n", filePath);
	}

	glGenTextures(1, &rendererID);
	glBindTexture(GL_TEXTURE_2D, rendererID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);

	glBindTexture(GL_TEXTURE_2D, 0);

	if (localBuffer)
	{
		SOIL_free_image_data(localBuffer);	//if the buffer does contain data then free it
	}
}

void Texture::GenerateTexture2D(int width, int height)
{
	unsigned char* texdata = new unsigned char[width * height * 3];

	for (int i = 0; i < width * height * 3; i++)
	{
		texdata[i] = 255;
	}

	glGenTextures(1, &rendererID);
	glBindTexture(GL_TEXTURE_2D, rendererID);

	// Set parameters to determine how the texture is resized
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set parameters to determine how the texture wraps at edges
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Read the texture data from file and upload it to the GPU
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texdata);

	glBindTexture(GL_TEXTURE_2D, 1);

	glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::GenerateTexture3D(int width, int height, int depth)
{
	unsigned char* texdata = new unsigned char[width * height * depth * 3];

	for (int i = 0; i < width * height * depth * 3; i++)
	{
		texdata[i] = 255;
	}

	glGenTextures(1, &rendererID);
	glBindTexture(GL_TEXTURE_3D, rendererID);

	// Set parameters to determine how the texture is resized
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set parameters to determine how the texture wraps at edges
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Read the texture data from file and upload it to the GPU
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0, GL_RGB, GL_UNSIGNED_BYTE, texdata);

	glBindTexture(GL_TEXTURE_3D, 1);

	glGenerateMipmap(GL_TEXTURE_3D);
}

void Texture::Bind(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, rendererID);
}

void Texture::Unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Bind3D(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_3D, rendererID);
}

void Texture::Unbind3D()
{
	glBindTexture(GL_TEXTURE_3D, 0);
}