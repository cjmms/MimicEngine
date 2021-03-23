#include "Scene.h"
#include "Core/Model.h"
#include "ResourceManager.h"
#include <sstream>


Scene::Scene()
{}


Scene::~Scene()
{
    lightSources.clear();
    objects.clear();
}

void Scene::addObjects(const char* address, glm::vec3 scale)
{
    Model* model = new Model(address);
    objects.push_back(new Object(model, scale));
}




void Scene::RenderShadowMap(glm::mat4 lightView, glm::mat4 lightProjection, Shader& shader) const
{
    shader.Bind();

    // pass projection and view matrix
    shader.setMat4("projection", lightProjection);
    shader.setMat4("view", lightView);

    //for (auto obj : objects) obj->Draw(shader);

    shader.unBind();
}








Object::Object(Model* model, glm::vec3 scale)
    : model(model), scale(scale)
{}


Object::~Object()
{
    delete model;
}



glm::mat4 Object::getModelMatrix() const
{
    return glm::scale(glm::mat4(1.0), scale);
}



void Scene::InitTextures(std::string str)
{
    std::string s = str + "albedo.jpg";
    albedo = ResourceManager::loadTexture(s.c_str());

    s = str + "normal.jpg";
    normal = ResourceManager::loadTexture(s.c_str());

    s = str + "roughness.jpg";
    roughness = ResourceManager::loadTexture(s.c_str());

    s = str + "metallic.jpg";
    metallic = ResourceManager::loadTexture(s.c_str());
}


void Scene::BindTextures(Shader* shader) const
{
    shader->setTexture("material.texture_albedo", albedo);
    shader->setTexture("material.texture_normal", normal);
    shader->setTexture("material.texture_metallic", metallic);
    shader->setTexture("material.texture_roughness", roughness);
}