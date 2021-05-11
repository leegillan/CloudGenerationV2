#include "Shader.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

Shader::Shader(const std::string& filePath)
	: filePath(filePath), rendererID(0)
{
	ShaderSource source = ParseShader(filePath);
	rendererID = CreateShader(source.vertexSource, source.fragmentSource);
	isBound = false;
}

Shader::~Shader()
{
	glDeleteProgram(rendererID);
}

ShaderSource Shader::ParseShader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class ShaderType
	{
		NONE = -1,
		VERTEX = 0,
		FRAGMENT = 1
	};

	std::string line;
	std::stringstream ss[2];
	ShaderType type = ShaderType::NONE;

	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)  //hasnt found it
		{
			if (line.find("vertex") != std::string::npos)
			{
				//set mode to vertex
				type = ShaderType::VERTEX;
			}
			else if (line.find("fragment") != std::string::npos)
			{
				//set mode to fragment
				type = ShaderType::FRAGMENT;
			}
		}
		else
		{
			ss[(int)type] << line << '\n';
		}
	}

	return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);

	const char* src = source.c_str();

	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);

	if (result == GL_FALSE)
	{
		int length;

		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

		char* message = (char*)_malloca(length * sizeof(char));

		glGetShaderInfoLog(id, length, &length, message);

		std::cout << "Failed to compile" << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << "shader!" << std::endl;
		std::cout << message << std::endl;

		glDeleteShader(id);
		return 0;
	}

	return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

void Shader::Bind()
{
	isBound = true;

	glUseProgram(rendererID);
}

void Shader::Unbind()
{
	isBound = false;
	glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string& name, float value)
{
	glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform2f(const std::string& name, glm::vec2 values)
{
	glUniform2f(GetUniformLocation(name), values.x, values.y);
}

void Shader::SetUniformMat4(const std::string& name, const glm::mat4 matrix)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::SetUniform4f(const std::string& name, glm::vec4 values)
{
	glUniform4f(GetUniformLocation(name), values.x, values.y, values.z, values.w);
}

int Shader::GetUniformLocation(const std::string& name)
{
	if (uniformLocationCache.find(name) != uniformLocationCache.end())
	{
		return uniformLocationCache[name];
	}

	int location = glGetUniformLocation(rendererID, name.c_str());

	if (location == -1)
	{
		std::cout << "Failure to find location of uniform " << name << ": doesn't exist." << std::endl;
	}

	uniformLocationCache[name] = location;

	return location;
}