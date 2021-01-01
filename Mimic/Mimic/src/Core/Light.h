#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "Model.h"
#include "Shader.h"

class Light
{
public:
	Light(glm::vec3 position, glm::vec3 intensity);
	~Light();
	
	void Draw(Shader& shader) const;

	inline glm::vec3 getPos() const { return position; }
	inline glm::vec3 getIntensity() const { return intensity; }

private:
	Model *lightSphere;
	glm::vec3 position;
	glm::vec3 intensity;

};

