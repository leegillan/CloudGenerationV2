#pragma once

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <unordered_map>

struct ShaderSource
{
	std::string vertexSource;
	std::string fragmentSource;
};

class Shader
{
public:
	Shader(const std::string& fileName);
	~Shader();

	void Bind();
	void Unbind();

	void SetUniform1i(const std::string& name, int value);
	void SetUniform1f(const std::string& name, float value);
	void SetUniform2f(const std::string& name, glm::vec2 values);
	void SetUniformMat4(const std::string& name, const glm::mat4 matrix);
	void SetUniform4f(const std::string& name, glm::vec4 values);

	bool isBound;

private:
	std::string filePath;

	unsigned int rendererID;

	//using unordered map for faster access as it is 0(1) (constant no matter size)
	//instead of 0(log(n)) (increases in time the more data)
	std::unordered_map<std::string, int> uniformLocationCache;

	ShaderSource ParseShader(const std::string& filepath);
	unsigned int CompileShader(unsigned int type, const std::string& source);
	unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader);

	int GetUniformLocation(const std::string& name);
};

