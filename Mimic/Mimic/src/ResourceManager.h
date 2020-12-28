#pragma once
#include "Core/IndexBuffer.h"
#include "Core/Shader.h"




class ResourceManager
{
public:
	bool init();

	void close();

	// maybe use sth to  keep track of loaded textures?
	static unsigned int loadTexture(char const* path, bool gamma = false);

	static unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

};

class Quad
{
public:
	Quad();

	void Draw(Shader& shader, unsigned int texture);

private:
	unsigned int VAO;
};

