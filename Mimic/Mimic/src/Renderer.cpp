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
	PBR_Forward_Shader(NULL), DeferredShader(NULL), Fill_G_Buffer(NULL), gBuffer(0),
    debugMode(debugMode)
{
	glEnable(GL_DEPTH_TEST);

    // init forward rendering shader
	PBR_Forward_Shader = new Shader("res/Shaders/ForwardPBR.shader");

    // init Deferred Rendering shader and G buffer
	Fill_G_Buffer = new Shader("res/Shaders/FillG-Buffer.shader");
	DeferredShader = new Shader("res/Shaders/DeferredPBR.shader");
	init_G_Buffer(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());

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




void Renderer::init_G_Buffer(unsigned int width, unsigned int height)
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Albedo + Metallic
    glGenTextures(1, &gAlbedoMetallic);
    glBindTexture(GL_TEXTURE_2D, gAlbedoMetallic);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedoMetallic, 0);

	// position
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gPosition, 0);

    // Normal + Roughness
    glGenTextures(1, &gNormalRoughness);
    glBindTexture(GL_TEXTURE_2D, gNormalRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gNormalRoughness, 0);


    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int G_Buffer[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, G_Buffer);

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}



void Renderer::RenderShadowMap(glm::mat4 view, glm::mat4 projection, Scene const* scene)
{
    depthBufferFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ShadowMapShader->setMat4("view", view);
    ShadowMapShader->setMat4("projection", projection);

    Draw(ShadowMapShader, scene);
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


    if (isDeferred()) DeferredRender(scene);
    else ForwardRender(PBR_Forward_Shader, scene);
    
    if (debugMode) {
        // For debugging purposes
        VisualizeDepthBuffer(depthBufferFBO->getDepthAttachment());
    }
}



void Renderer::Draw(Shader* shader, Scene const* scene) const
{
    for (auto obj : scene->getObjects())
    {
        shader->setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(*shader);
    }
}




void Renderer::ForwardRender(Shader* shader, Scene const* scene) const
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // pass light source
    BindLightSources(shader, scene);

    bindShadowMap(shader);

    // pass projection and view matrix
    shader->setMat4("projection", camera.getProjectionMatrix());
    shader->setMat4("view", camera.getViewMatrix());

    // pass camera position
    shader->setVec3("camPos", camera.getCameraPos());

    Draw(shader, scene);
}



void Renderer::FillG_Buffer(Scene const* scene) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);     // Bind to G-Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Fill_G_Buffer->setMat4("projection", camera.getProjectionMatrix());
    Fill_G_Buffer->setMat4("view", camera.getViewMatrix());

    Draw(Fill_G_Buffer, scene);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);           // Unbind G-Buffer
}



void Renderer::BindG_Buffer(Shader* shader) const
{
    shader->setTexture("gPosition", gPosition);
    shader->setTexture("gAlbedoMetallic", gAlbedoMetallic);
    shader->setTexture("gNormalRoughness", gNormalRoughness);
}


void Renderer::DeferredRender(Scene const* scene) const
{
    // First Pass, fill G-Buffer
    FillG_Buffer(scene);
    
    VolumetricLight(LightingFBO);

    // bind shadow map 
    bindShadowMap(DeferredShader);

    // bind volumetric texture
    DeferredShader->setTexture("volumetricLightTexture", LightingFBO->getColorAttachment());

    // Lighting Calculation
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    BindG_Buffer(DeferredShader);
    BindLightSources(DeferredShader, scene);
    DeferredShader->setVec3("camPos", camera.getCameraPos());
    
    Quad().Draw(*DeferredShader);
}


void Renderer::VolumetricLight(FBO_Color* fbo) const
{
    // half resolution ray marching
    HalfResFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bindShadowMap(VolumetricLightShader);

    //BindG_Buffer(VolumetricLightShader);
    VolumetricLightShader->setTexture("gPosition", gPosition);

    VolumetricLightShader->setVec3("camPos", camera.getCameraPos());

    Quad().Draw(*VolumetricLightShader);

    HalfResFBO->Unbind();


    fbo->Bind();
    // Bilateral Upsampling to resolution
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    BilateralUpShader->setTexture("volumetricLightTexture", HalfResFBO->getColorAttachment());
    BilateralUpShader->setTexture("gPosition", gPosition);

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



void Renderer::BindLightSources(Shader* shader, Scene const* scene) const
{
	const std::vector<Light* > lights = scene->getLightSources();

	for (unsigned int i = 0; i < lights.size(); i++)
	{
		shader->setVec3("lightPositions[" + std::to_string(i) + "]", lights[i]->getPos());
		shader->setVec3("lightColors[" + std::to_string(i) + "]", lights[i]->getIntensity());
	}
}