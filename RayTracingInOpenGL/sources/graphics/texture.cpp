#include "texture.h"

Texture::Texture(int width, int height, int internalFormat, int format, int type) 
	: ID()
{
	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(int unit)
{
	if (unit >= 0 && unit <= 15)
	{
		glActiveTexture(GL_TEXTURE0 + unit);
		glBindTexture(GL_TEXTURE_2D, ID);
	}
	else
	{
		std::cout << "[ERROR] TEXTURE: Failed to bind texture in " << unit << " unit." << std::endl;
	}
}

void Texture::bindImage(int unit, int access, int format)
{
	if (unit >= 0 && unit <= 15)
	{
		glBindImageTexture(unit, ID, 0, GL_FALSE, 0, access, format);
	}
	else
	{
		std::cout << "[ERROR] TEXTURE: Failed to bind image texture in " << unit << " unit." << std::endl;
	}
}

void Texture::unbind()
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
