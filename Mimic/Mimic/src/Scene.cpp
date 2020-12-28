#include "Scene.h"



void Scene::BindLightSources(Shader& shader)
{
    shader.Bind();
    for (unsigned int i = 0; i < lightSources.size(); i++)
    {
        shader.setVec3("lightPositions[" + std::to_string(i) + "]", lightSources[i]->getPos());
        shader.setVec3("lightColors[" + std::to_string(i) + "]", lightSources[i]->getIntensity());
    }
    shader.unBind();
}


Scene::Scene()
{
    lightShader = new Shader("res/Shaders/basic.shader");
}


Scene::~Scene()
{
    delete lightShader;
    lightSources.clear();
}