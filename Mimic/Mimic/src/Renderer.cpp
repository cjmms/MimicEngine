#include "Renderer.h"
#include "Core/Camera.h"
#include "UI_Manager.h"
#include <iostream>
#include "ResourceManager.h"
#include "Core/FBO.h"

extern Camera camera;
extern UI_Manager UI_Mgr;

Renderer::Renderer(RenderingType type)
	:type(type), lightShader(new Shader("res/Shaders/basic.shader")),
	PBR_Forward_Shader(NULL), DeferredShader(NULL), Fill_G_Buffer(NULL), gBuffer(0)
{
	glEnable(GL_DEPTH_TEST);

    // init forward rendering shader
	PBR_Forward_Shader = new Shader("res/Shaders/model_loading.shader");

    // init Deferred Rendering shader and G buffer
	Fill_G_Buffer = new Shader("res/Shaders/FillG-Buffer.shader");
	DeferredShader = new Shader("res/Shaders/DeferredPBR.shader");
	init_G_Buffer(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
}




void Renderer::init_G_Buffer(unsigned int width, unsigned int height)
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    //unsigned int gPosition, gNormalRoughness, gAlbedoMetallic;

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

    //DeferredShader->Bind();
    // TODO: need to be done in a better way
   // DeferredShader->setTexture("gPosition", gPosition, 6);
   // DeferredShader->setTexture("gAlbedoMetallic", gAlbedoMetallic, 7);
    //DeferredShader->setTexture("gNormalRoughness", gNormalRoughness, 8);

}


void Renderer::passDepthMap(Shader* shader) 
{
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-60.0f, 70.0f, 0.0f),
        glm::vec3(30.0f, 60.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // width / height is 1.0, front plane: 0.1f, back plane 125.0f
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);

    shader->Bind();
    shader->setMat4("lightView", lightView);
    shader->setMat4("lightProjection", lightProjection);

    shader->setInt("shadowMap", 4);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, depthBufferFBO->getDepthAttachment());

    shader->unBind();
}




void Renderer::setDepthMap(Scene const* scene)
{
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-60.0f, 70.0f, 0.0f),
        glm::vec3(30.0f, 60.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // width / height is 1.0, front plane: 0.1f, back plane 125.0f
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);

    Shader ShadowMapShader("res/Shaders/DepthMap.shader");

    depthBufferFBO = new FBO_Depth(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());

    depthBufferFBO->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    // pass projection and view matrix
    ShadowMapShader.setMat4("projection", lightProjection);
    ShadowMapShader.setMat4("view", lightView);

    Draw(&ShadowMapShader, scene);

    depthBufferFBO->Unbind();
}



Renderer::~Renderer()
{}



void Renderer::Render(Scene const* scene) 
{
    if (isDeferred()) DeferredRender(scene);
    else 
    {
        //passDepthMap(PBR_Forward_Shader);

        RenderObj(PBR_Forward_Shader, scene);

        //Shader depthQuadShader("res/Shaders/DepthQuad.shader");
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Quad::Quad().Draw(depthQuadShader, depthBufferFBO->getDepthAttachment());

        glm::mat4 lightView = glm::lookAt(
            glm::vec3(-60.0f, 70.0f, 0.0f),
            glm::vec3(30.0f, 60.0f, 55.0f),
            glm::vec3(0.0f, 1.0f, 0.0f));


        glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);
        /*
        Shader ShadowMapShader("res/Shaders/DepthMap.shader");

        //FBO_Color FBO(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());

        //FBO.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        PBR_Forward_Shader->Bind();
        // pass projection and view matrix
        PBR_Forward_Shader->setMat4("projection", lightProjection);
        PBR_Forward_Shader->setMat4("view", lightView);

        PBR_Forward_Shader->setVec3("camPos", camera.getCameraPos());

        for (auto obj : scene->getObjects())
        {

            PBR_Forward_Shader->setMat4("model", obj->getModelMatrix());

            obj->getModel()->Draw(*PBR_Forward_Shader);
        }
        PBR_Forward_Shader->unBind();
        */
        //--------------------------------------------------------------------------------------
        /*
        Shader shader("res/Shaders/DepthMap.shader");

        FBO_Color FBO(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
        FBO.Bind();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        PBR_Forward_Shader->Bind();

        // pass light source
        BindLightSources(PBR_Forward_Shader, scene);

        PBR_Forward_Shader->setMat4("projection", camera.getProjectionMatrix());
        PBR_Forward_Shader->setMat4("view", camera.getViewMatrix());
        // pass projection and view matrix
        //PBR_Forward_Shader->setMat4("projection", lightProjection);
        //PBR_Forward_Shader->setMat4("view", lightView);

        // pass camera position
        PBR_Forward_Shader->setVec3("camPos", camera.getCameraPos());

        for (auto obj : scene->getObjects())
        {
            PBR_Forward_Shader->setMat4("model", obj->getModelMatrix());
            obj->getModel()->Draw(*PBR_Forward_Shader);
        }
        PBR_Forward_Shader->unBind();
        FBO.Unbind();

        //ShadowMapShader.unBind();

        Shader quadShader("res/Shaders/ColorQuad.shader");
        Shader depthQuadShader("res/Shaders/DepthQuad.shader");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Quad::Quad().Draw(depthQuadShader, FBO.depthBuffer);
        */
        //----------------------------------------------------------------------------------------
        
        /*
        // configure depth map FBO
        // -----------------------
        glEnable(GL_DEPTH_TEST);

        const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
        unsigned int depthMapFBO;
        glGenFramebuffers(1, &depthMapFBO);
        // create depth texture
        unsigned int depthMap;
        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // attach depth texture as FBO's depth buffer
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Shader shader("res/Shaders/DepthMap.shader");

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Bind();

        shader.setMat4("projection", camera.getProjectionMatrix());
        shader.setMat4("view", camera.getViewMatrix());


        for (auto obj : scene->getObjects())
        {
            shader.setMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)));
            obj->getModel()->Draw(shader);
        }
        shader.unBind();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        Shader quadShader("res/Shaders/ColorQuad.shader");
        Shader depthQuadShader("res/Shaders/DepthQuad.shader");
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Quad::Quad().Draw(depthQuadShader, depthMap);
        */
        
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




void Renderer::RenderObj(Shader* shader, Scene const* scene) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // pass light source
    BindLightSources(shader, scene);

    // pass projection and view matrix
    shader->setMat4("projection", camera.getProjectionMatrix());
    shader->setMat4("view", camera.getViewMatrix());

    // pass camera position
    shader->setVec3("camPos", camera.getCameraPos());

    Draw(shader, scene);
}








void Renderer::DeferredRender(Scene const* scene) const
{
    // First Pass, fill G-Buffer
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);     // Bind to G-Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Fill_G_Buffer->setMat4("projection", camera.getProjectionMatrix());
    Fill_G_Buffer->setMat4("view", camera.getViewMatrix());

    Draw(Fill_G_Buffer, scene);

    // Second Buffer, Render to a Quad
    glBindFramebuffer(GL_FRAMEBUFFER, 0);           // Unbind G-Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DeferredShader->setTexture("gPosition", gPosition);
    DeferredShader->setTexture("gAlbedoMetallic", gAlbedoMetallic);
    DeferredShader->setTexture("gNormalRoughness", gNormalRoughness);

    BindLightSources(DeferredShader, scene);
    DeferredShader->setVec3("camPos", camera.getCameraPos());


    Quad().Draw(*DeferredShader);
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