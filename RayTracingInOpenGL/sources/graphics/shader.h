#pragma once

#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

class ShaderProgram
{
public:
	ShaderProgram(const char* vsFilepath, const char* fsFilepath);
	ShaderProgram(const char* csFilepath);

	void bind();
	void unbind();

	void setUniform1i(const char* uniformName, int data);
	void setUniform1f(const char* uniformName, float data);

private:
	unsigned int ID;

	unsigned int createShader(const char* sFilepath, int shaderType);
};
