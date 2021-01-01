#pragma once
#include <vector>
#include "Core/Light.h"


struct Object
{
	Model* model;
	glm::vec3 scale;

	Object(Model* model, glm::vec3 scale);
	~Object();
	void Draw(Shader& shader) const;
	
};


class Scene
{
public:
	Scene();
	~Scene();

	inline void addLightSource(glm::vec3 position, glm::vec3 intensity) { lightSources.push_back(new Light(position, intensity)); }
	inline void RenderLightSources() const { for (auto light : lightSources) light->Draw(*lightShader); }

	void addObjects(const char* address, glm::vec3 scale);
	void RenderObjects(Shader& shader) const;

	void RenderShadowMap(glm::mat4 lightView, glm::mat4 lightProjection, Shader& shader) const;
	void BindLightSources(Shader& shader) const;


private:
	std::vector<Light* > lightSources;
	std::vector<Object* > objects;
	Shader *lightShader;

	// this func will not bind and unbind shader, it only passes uniforms
	//void BindLightSources(Shader& shader) const;

};

