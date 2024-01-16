#pragma once

#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ShaderProgram
{
public:
	ShaderProgram(const char* vsFilepath, const char* fsFilepath);
	ShaderProgram(const char* csFilepath);

	void bind();
	void unbind();

	void setUniform1i(const char* uniformName, int data);
	void setUniform1f(const char* uniformName, float data);
	void setUniform3f(const char* uniformName, const glm::vec3& data);
	void setUniformMatrix4fv(const char* uniformName, const glm::mat4& data);

private:
	unsigned int ID;

	unsigned int createShader(const char* sFilepath, int shaderType);
};
