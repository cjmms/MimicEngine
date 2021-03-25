#include "Scene.h"
#include "Core/Model.h"
#include "ResourceManager.h"
#include <sstream>


Scene::Scene()
{
    initPlane();
}


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




void Scene::initPlane()
{
    float planeVertices[] = {
        // positions            // normals         // texcoords
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
    };

    unsigned int VBO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &planeVAO);
    glBindVertexArray(planeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}








void Scene::RenderPlane(Shader* shader) const
{
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(10));
    shader->setMat4("model", model);

    shader->Bind();
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    shader->unBind();
}
