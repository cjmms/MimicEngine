#pragma once
#include <vector>
#include "Core/Light.h"

class Scene
{
public:
	Scene();
	~Scene();

	inline void addLightSource(glm::vec3 position, glm::vec3 intensity) { lightSources.push_back(new Light(position, intensity)); }
	inline void RenderLightSources() { for (auto light : lightSources) light->Draw(*lightShader); }

	void BindLightSources(Shader& shader);

private:
	std::vector<Light* > lightSources;
	Shader *lightShader;

};

