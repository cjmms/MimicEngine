#include "Renderer.h"
#include "Core/Camera.h"
#include "UI_Manager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Core/FBO.h"
#include "Scene.h"
#include "glm/gtc/matrix_transform.hpp"

extern bool test;
extern Camera camera;
extern UI_Manager UI_Mgr;

Renderer::Renderer(bool debugMode)
    :lightShader(new Shader("res/Shaders/basic.shader")),
    debugMode(debugMode),
    DeferredRenderer(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight()),
    VolumetricLight(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight())
{
	glEnable(GL_DEPTH_TEST);

    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-70.0f, 70.0f, -10.0f), glm::vec3(30.0f, 60.0f, 55.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);

    shadow = new Shadow(lightView, lightProjection, 
        UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
  
    ColorQuadShader = new Shader("res/Shaders/ColorQuad.shader");

    DepthQuadShader = new Shader("res/Shaders/DepthQuad.shader");

    ForwardShader = new Shader("res/Shaders/ForwardPBR.shader");
}




void Renderer::VisualizeDepthBuffer(unsigned int depthAttachment) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Quad::Quad().Draw(*DepthQuadShader, depthAttachment);
}


Renderer::~Renderer()
{}



void Renderer::Render(Scene const* scene)  
{
    //shadow->CalculateShadowMap(scene);
    //shadow->CalculateMSM(scene);

    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Quad().Draw(*ColorQuadShader, shadow->GetMSM());
    //Quad().Draw(*ColorQuadShader, shadow->GetShadowMap());

    
    // First Pass, fill G-Buffer
    //DeferredRenderer.Fill_G_Buffer(scene);


    //VolumetricLight.Compute(*shadow, DeferredRenderer.Get_G_Position());


    //DeferredRenderer.BindShadowMap(*shadow);
    //DeferredRenderer.BindMSM(*shadow);

    //DeferredRenderer.BindVolumetricLight(VolumetricLight);
    //DeferredRenderer.Render(scene);
    

    ForwardRendering(scene);
    
   // if (debugMode) {
         //For debugging purposes
        //VisualizeDepthBuffer(shadow->GetShadowMap());
    //}
}

void Renderer::ForwardRendering(Scene const* scene)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const std::vector<Light* > lights = scene->getLightSources();

    for (unsigned int i = 0; i < lights.size(); i++)
    {
        ForwardShader->setVec3("lightPositions[" + std::to_string(i) + "]", lights[i]->getPos());
        ForwardShader->setVec3("lightColors[" + std::to_string(i) + "]", lights[i]->getIntensity());
    }

    ForwardShader->setVec3("camPos", camera.getCameraPos());

    ForwardShader->setMat4("view", camera.getViewMatrix());
    ForwardShader->setMat4("projection", camera.getProjectionMatrix());

    for (auto obj : scene->getObjects())
    {
        ForwardShader->setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(*ForwardShader);
    }
}



void Renderer::RenderLightSources(Scene const* scene) const
{
	 //glDisable(GL_DEPTH_TEST);

    lightShader->setMat4("projection", camera.getProjectionMatrix());
    lightShader->setMat4("view", camera.getViewMatrix());

	// This is a bad design, since Light* can still be changed
	// The right way should be passing a pair of iterators instead ( Maybe? )
	const std::vector<Light* > lights = scene->getLightSources();
    for (auto light : lights) {
        lightShader->setMat4("model", light->getModelMatrix());

        light->getModel()->Draw(*lightShader);
    }

	 //glEnable(GL_DEPTH_TEST);
}




void Renderer::RenderPlane() const
{
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    unsigned int quadVBO, quadVAO;

    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    lightShader->setMat4("projection", camera.getProjectionMatrix());
    lightShader->setMat4("view", camera.getViewMatrix());

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(20));
    model = glm::rotate(model, 3.14f / 2.0f, glm::vec3(1.0, 0.0, 0.0));
    //model = glm::translate(model, glm::vec3(0.0, 0.0, 0.1));
    lightShader->setMat4("model", model);


    lightShader->Bind();

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    lightShader->unBind();
}