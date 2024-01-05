#pragma once

#include <iostream>

#include <glad/glad.h>

class Texture
{
public:
	Texture(int width, int height, int internalFormat, int format, int type);

	void bind(int unit);
	void bindImage(int unit, int access, int format);

	void unbind();

private:
	unsigned int ID;
};
