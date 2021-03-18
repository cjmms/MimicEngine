#include "Renderer.h"
#include "Core/Camera.h"
#include "UI_Manager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Core/FBO.h"

extern bool test;
extern Camera camera;
extern UI_Manager UI_Mgr;

Renderer::Renderer(RenderingType type, bool debugMode)
	:type(type), lightShader(new Shader("res/Shaders/basic.shader")),
	 debugMode(debugMode), 
    DeferredRenderer(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight())
{
	glEnable(GL_DEPTH_TEST);


    // init shader and fbo for shadow mapping
    ShadowMapShader = new Shader("res/Shaders/DepthMap.shader");
    depthBufferFBO = new FBO_Depth(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());

    BilateralUpShader = new Shader("res/Shaders/BilateralUp.shader");

    VolumetricLightShader = new Shader("res/Shaders/VolumetricLighting.shader");

    HalfResFBO = new FBO_Color(UI_Mgr.getScreenWidth() / 2.0f, UI_Mgr.getScreenHeight() / 2.0f);

    LightingFBO = new FBO_Color(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());

    ColorQuadShader = new Shader("res/Shaders/ColorQuad.shader");

    if (debugMode) {
        DepthQuadShader = new Shader("res/Shaders/DepthQuad.shader");
    }

}





void Renderer::RenderShadowMap(glm::mat4 view, glm::mat4 projection, Scene const* scene)
{
    depthBufferFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ShadowMapShader->setMat4("view", view);
    ShadowMapShader->setMat4("projection", projection);

    //Draw(ShadowMapShader, scene);
    depthBufferFBO->Unbind();
}


void Renderer::bindShadowMap(Shader* shader) const
{
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-70.0f, 70.0f, -10.0f), glm::vec3(30.0f, 60.0f, 55.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);


    // bind light matrix
    shader->setMat4("lightProjection", lightProjection);
    shader->setMat4("lightView", lightView);

    // bind shadow map
    shader->setTexture("shadowMap", depthBufferFBO->getDepthAttachment());
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
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-70.0f, 70.0f, -10.0f), glm::vec3(30.0f, 60.0f, 55.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);
    

    RenderShadowMap(lightView, lightProjection, scene);


    DeferredRender(scene);
    
    if (debugMode) {
        // For debugging purposes
        VisualizeDepthBuffer(depthBufferFBO->getDepthAttachment());
    }
}




void Renderer::DeferredRender(Scene const* scene) const
{
    // First Pass, fill G-Buffer
    //FillG_Buffer(scene);
    DeferredRenderer.Fill_G_Buffer(scene);

    //VolumetricLight(LightingFBO);

    // bind shadow map 
    //bindShadowMap(DeferredShader);

    // bind volumetric texture
    //DeferredShader->setTexture("volumetricLightTexture", LightingFBO->getColorAttachment());


    DeferredRenderer.Render(scene);
}


void Renderer::VolumetricLight(FBO_Color* fbo) const
{
    // half resolution ray marching
    HalfResFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bindShadowMap(VolumetricLightShader);

    //BindG_Buffer(VolumetricLightShader);
    //VolumetricLightShader->setTexture("gPosition", gPosition);

    VolumetricLightShader->setVec3("camPos", camera.getCameraPos());

    Quad().Draw(*VolumetricLightShader);

    HalfResFBO->Unbind();


    fbo->Bind();
    // Bilateral Upsampling to resolution
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    BilateralUpShader->setTexture("volumetricLightTexture", HalfResFBO->getColorAttachment());
    //BilateralUpShader->setTexture("gPosition", gPosition);

    BilateralUpShader->setInt("BilateralSwitch", test);

    Quad().Draw(*BilateralUpShader);
    fbo->Unbind();
}



void Renderer::RenderLightSources(Scene const* scene) const
{
	if (isDeferred()) glDisable(GL_DEPTH_TEST);

    lightShader->setMat4("projection", camera.getProjectionMatrix());
    lightShader->setMat4("view", camera.getViewMatrix());

	// This is a bad design, since Light* can still be changed
	// The right way should be passing a pair of iterators instead ( Maybe? )
	const std::vector<Light* > lights = scene->getLightSources();
    for (auto light : lights) {
        lightShader->setMat4("model", light->getModelMatrix());

        light->getModel()->Draw(*lightShader);
    }

	if (isDeferred()) glEnable(GL_DEPTH_TEST);
}
