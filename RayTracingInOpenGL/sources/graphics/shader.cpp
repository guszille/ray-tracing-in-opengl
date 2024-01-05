#include "shader.h"

ShaderProgram::ShaderProgram(const char* vsFilepath, const char* fsFilepath)
	: ID()
{
	int success;
	char infoLog[512];

	unsigned int vsID = createShader(vsFilepath, GL_VERTEX_SHADER);
	unsigned int fsID = createShader(fsFilepath, GL_FRAGMENT_SHADER);

	ID = glCreateProgram();

	glAttachShader(ID, vsID);
	glAttachShader(ID, fsID);
		glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);

		std::cout << "[ERROR] SHADER PROGRAM: Linkage failed!\n" << infoLog << std::endl;
	}

	glDeleteShader(vsID);
	glDeleteShader(fsID);
}

ShaderProgram::ShaderProgram(const char* csFilepath)
	: ID()
{
	int success;
	char infoLog[512];

	unsigned int csID = createShader(csFilepath, GL_COMPUTE_SHADER);

	ID = glCreateProgram();

	glAttachShader(ID, csID);
		glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);

	if (!success)
	{
		glGetProgramInfoLog(ID, 512, NULL, infoLog);

		std::cout << "[ERROR] SHADER PROGRAM: Linkage failed!\n" << infoLog << std::endl;
	}

	glDeleteShader(csID);
}

void ShaderProgram::bind()
{
	glUseProgram(ID);
}

void ShaderProgram::unbind()
{
	glUseProgram(0);
}

void ShaderProgram::setUniform1i(const char* uniformName, int data)
{
	int uniformLocation = glGetUniformLocation(ID, uniformName);

	if (uniformLocation > -1)
	{
		glUniform1i(uniformLocation, data);
	}
	else
	{
		std::cout << "[ERROR] SHADER PROGRAM: Failed to get location of uniform \"" << uniformName << "\"." << std::endl;
	}
}

void ShaderProgram::setUniform1f(const char* uniformName, float data)
{
	int uniformLocation = glGetUniformLocation(ID, uniformName);

	if (uniformLocation > -1)
	{
		glUniform1f(uniformLocation, data);
	}
	else
	{
		std::cout << "[ERROR] SHADER PROGRAM: Failed to get location of uniform \"" << uniformName << "\"." << std::endl;
	}
}

unsigned int ShaderProgram::createShader(const char* sFilepath, int shaderType)
{
	int success;
	char infoLog[512];

	std::ifstream fileStream(sFilepath);
	std::stringstream stringStream; stringStream << fileStream.rdbuf();
	std::string shaderSource = stringStream.str();

	const char* shaderCode = shaderSource.c_str();
	unsigned int shaderID = glCreateShader(shaderType);

	glShaderSource(shaderID, 1, &shaderCode, NULL);
		glCompileShader(shaderID);
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(shaderID, 512, NULL, infoLog);

		std::cout << "[ERROR] SHADER PROGRAM: Compilation failed!\n" << infoLog << std::endl;

		glDeleteShader(shaderID);

		return -1;
	}

	return shaderID;
}
