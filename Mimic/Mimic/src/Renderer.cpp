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
    VolumetricLight(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight()),
    PingBufferFBO(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight()),
    PongBufferFBO(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight())
{
	glEnable(GL_DEPTH_TEST);
    /*
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-70.0f, 70.0f, -10.0f), glm::vec3(30.0f, 60.0f, 55.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);
    */

    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-15.0f, 15.0f, 10.0f), glm::vec3(0.0), glm::vec3(0.0, 1.0, 0.0));

    glm::mat4 lightProjection = camera.getProjectionMatrix();

    shadow = new Shadow(lightView, lightProjection, 
        UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
  
    ColorQuadShader = new Shader("res/Shaders/ColorQuad.shader");

    DepthQuadShader = new Shader("res/Shaders/DepthQuad.shader");

    ForwardShader = new Shader("res/Shaders/ForwardPBR.shader");

    GaussianBlurShader = new Shader("res/Shaders/GaussianBlur.shader");

    initPlane();
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
    //shadow->CalculateMSM(scene);

    //VisualizeDepthBuffer(blurred);
    
    // First Pass, fill G-Buffer
    scene->BindTextures(DeferredRenderer.GetFillBufferShader());       

    DeferredRenderer.Fill_G_Buffer(scene);

    //VolumetricLight.Compute(*shadow, DeferredRenderer.Get_G_Position());


    //DeferredRenderer.BindShadowMap(*shadow);
    //DeferredRenderer.BindMSM(*shadow);

    //DeferredRenderer.BindVolumetricLight(VolumetricLight);
    DeferredRenderer.Render(scene);
    
    
    //ForwardRendering(scene);    
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

    ForwardShader->setMat4("lightProjection", shadow->GetProjection());
    ForwardShader->setMat4("lightView", shadow->GetLightView());

    ForwardShader->setTexture("MSM", GaussianBlur(shadow->GetMSM(), 10));
    //ForwardShader->setTexture("MSM", shadow->GetMSM());
    ForwardShader->setTexture("ShadowMap", shadow->GetShadowMap());


    for (auto obj : scene->getObjects())
    {
        ForwardShader->setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(*ForwardShader);
    }

    RenderPlane();
}



void Renderer::initPlane()
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

    glGenVertexArrays(1, &quadVAO);
    glBindVertexArray(quadVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
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
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(10));
    ForwardShader->setMat4("model", model);

    ForwardShader->Bind();
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    ForwardShader->unBind();
}




unsigned int Renderer::GaussianBlur(unsigned int texture, int level) const
{
    // check if current FBO is as same as the input texture
    if (texture != PingBufferFBO.getColorAttachment())
    {
        PingBufferFBO.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Quad().Draw(*ColorQuadShader, texture);
        PingBufferFBO.Unbind();
    }

    for (int i = 0; i < level; i++)
    {

        PongBufferFBO.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GaussianBlurShader->setInt("horizontal", true);
        Quad().Draw(*GaussianBlurShader, PingBufferFBO.getColorAttachment());
        PongBufferFBO.Unbind();


        PingBufferFBO.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GaussianBlurShader->setInt("horizontal", false);
        Quad().Draw(*GaussianBlurShader, PongBufferFBO.getColorAttachment());
        PingBufferFBO.Unbind();
    }

    return PingBufferFBO.getColorAttachment();
}