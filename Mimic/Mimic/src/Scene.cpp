#include "Scene.h"
#include "Core/Model.h"

#include "Core/Camera.h"

extern Camera camera;

void Scene::BindLightSources(Shader& shader) const
{
    
    //for (unsigned int i = 0; i < lightSources.size(); i++)
   // {
       // shader.setVec3("lightPositions[" + std::to_string(i) + "]", lightSources[i]->getPos());
      //  shader.setVec3("lightColors[" + std::to_string(i) + "]", lightSources[i]->getIntensity());
    //}
    
    shader.setVec3("lightPositions[0]", glm::vec3(-5.0f, 15.0f, 10.0f));
    shader.setVec3("lightColors[0]", glm::vec3(550.0f, 550.0f, 550.0f));
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

void Scene::addObjects(const char* address, glm::vec3 scale)
{
    Model* model = new Model(address);


    objects.push_back(new Object(model, scale));
}

void Scene::RenderObjects(Shader& shader) const
{ 
    shader.Bind();

    // pass light source
    BindLightSources(shader);

    // pass projection and view matrix
    shader.setMat4("projection", camera.getProjectionMatrix());
    shader.setMat4("view", camera.getViewMatrix());

    // pass camera position
    shader.setVec3("camPos", camera.getCameraPos());

    for (auto obj : objects) obj->Draw(shader); 

    shader.unBind();
}




void Scene::RenderShadowMap(glm::mat4 lightView, glm::mat4 lightProjection, Shader& shader) const
{
    shader.Bind();

    // pass projection and view matrix
    shader.setMat4("projection", lightProjection);
    shader.setMat4("view", lightView);

    for (auto obj : objects) obj->Draw(shader);

    shader.unBind();
}








Object::Object(Model* model, glm::vec3 scale)
    : model(model), scale(scale)
{}


Object::~Object()
{
    delete model;
}


void Object::Draw(Shader& shader) const
{
    shader.Bind();

    // bind model matrix
    glm::mat4 m = glm::scale(glm::mat4(1.0), scale);
    shader.setMat4("model", m);
    
    model->Draw(shader);

    shader.unBind();
}



