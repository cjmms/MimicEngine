#pragma once
#include "Core/IndexBuffer.h"
#include "Core/Shader.h"


class ResourceManager
{
public:
	bool init();

	void close();



};

class Rectangle 
{
public:
	Rectangle();

	void Draw(Shader& shader);

private:
	unsigned int VAO;
};