#include "Renderer.h"
#include "Core/Camera.h"
#include "UI_Manager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Core/FBO.h"
#include "Scene.h"
#include "glm/gtc/matrix_transform.hpp"

#include "imgui/imgui.h"

//#include "imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"



/*

extern bool test;
extern Camera camera;
extern UI_Manager UI_Mgr;

int GaussianBlurPass = 0;
int ShadowMapType = 0;

Renderer::Renderer(Scene const* scene)
    :lightShader(new Shader("res/Shaders/basic.shader")),
    debugMode(debugMode),
    DeferredRenderer(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight()),
    VolumetricLight(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight()),
    PingBufferFBO(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight()),
    PongBufferFBO(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight()),
    IBL("res/IBL/Barce_Rooftop_C_3k.hdr", 4096),
    SSAO(128, 4), SSAOBlur(0)
{

    //std::cout <<  "Renderer Constructor" << std::endl;
	glEnable(GL_DEPTH_TEST);

    
    //shadow for Project 2
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-20.0f, 35.0f, 10.0),glm::vec3(0.0f, 0.0f, 00.0f), glm::vec3(0.0, 1.0, 0.0));

    glm::mat4 lightProjection = glm::perspective(
        glm::radians(45.0f), (float)UI_Mgr.getScreenWidth() / UI_Mgr.getScreenHeight(), 0.1f, 80.0f);

    shadow = new Shadow(lightView, lightProjection, UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
    
    
    // shadow for project 5
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-70.0f, 70.0f, -10.0f), glm::vec3(30.0f, 60.0f, 55.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);

    shadow = new Shadow(lightView, lightProjection, UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
    

    ColorQuadShader = new Shader("res/Shaders/ColorQuad.shader");

    DepthQuadShader = new Shader("res/Shaders/DepthQuad.shader");

    ForwardShader = new Shader("res/Shaders/ForwardPBR.shader");

    GaussianBlurShader = new Shader("res/Shaders/GaussianBlur.shader");

    //shadow->Compute(scene);
    
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
    RenderUI();

    // First Pass, fill G-Buffer
    scene->BindTextures(DeferredRenderer.GetFillBufferShader());       
    DeferredRenderer.Fill_G_Buffer(scene);

    

    SSAO.RenderSSAO(DeferredRenderer.Get_G_Position(), DeferredRenderer.Get_G_NormalRoughness());

    //VolumetricLight.Compute(*shadow, DeferredRenderer.Get_G_Position());


    DeferredRenderer.BindSSAO(GaussianBlur(SSAO.GetSSAO(), SSAOBlur));

    //DeferredRenderer.BindVolumetricLight(VolumetricLight);

    DeferredRenderer.Render(scene);

    //ForwardRendering(scene); 

    //IBL.RenderSkybox();

    glBindFramebuffer(GL_READ_FRAMEBUFFER, DeferredRenderer.Get_G_Buffer());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
    glBlitFramebuffer(
        0, 0, UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight(), 0, 0, UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight(), GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RenderLightSources(scene);
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

    ForwardShader->setTexture("ShadowMap", GaussianBlur(shadow->GetShadowMap(), GaussianBlurPass));
    ForwardShader->setInt("ShadowMapType", ShadowMapType);

    for (auto obj : scene->getObjects())
    {
        ForwardShader->setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(*ForwardShader);
    }

    scene->RenderPlane(ForwardShader);
}







void Renderer::RenderLightSources(Scene const* scene) const
{
	//glDisable(GL_DEPTH_TEST);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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


void Renderer::RenderUI()
{
    ImGui::Begin("UI");

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    //ShadowUI();
    //VolumetricLight.UI();
    DeferredRenderer.UI();

    ImGui::SliderInt("SSAO Blur Level", &SSAOBlur, 0, 10);

    ImGui::End();
}



void Renderer::ShadowUI()
{
    ImGui::Text("ShadowMap Gaussian Blur Pass.");
    ImGui::SliderInt("int", &GaussianBlurPass, 0, 10);

    ImGui::RadioButton("Standard Shadow Map", &ShadowMapType, 0);
    ImGui::RadioButton("Variance Shadow Map", &ShadowMapType, 1);
    ImGui::RadioButton("Moment Shadow Map", &ShadowMapType, 2);
}



*/


void Renderer::Update(float deltaTime)
{

}