#include "DeferredRendering.h"
#include <iostream>
#include "../Core/Camera.h"
#include "../ResourceManager.h"
#include "../Scene.h"
#include "../Shadow/Shadow.h"
#include "../VolumetricLight/VolumetricLight.h"

extern Camera camera;


DeferredRendering::DeferredRendering(unsigned int width, unsigned int height)
    :IBL("res/IBL/fin4_Ref.hdr", 4096)
{
    Fill_G_BufferShader = new Shader("res/Shaders/DeferredShading/FillG-Buffer.shader");
    DeferredLightingShader = new Shader("res/Shaders/DeferredShading/DeferredPBR.shader");
	init_G_Buffer(width, height);
}



void DeferredRendering::init_G_Buffer(unsigned int width, unsigned int height)
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



void DeferredRendering::Fill_G_Buffer(Scene const* scene) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);     // Bind to G-Buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Fill_G_BufferShader->setMat4("projection", camera.getProjectionMatrix());
    Fill_G_BufferShader->setMat4("view", camera.getViewMatrix());

    for (auto obj : scene->getObjects())
    {
        Fill_G_BufferShader->setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(*Fill_G_BufferShader);
    }

    IBL.RenderSkybox();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void DeferredRendering::Bind_G_Buffer(Shader* shader) const
{
    shader->setTexture("gPosition", gPosition);
    shader->setTexture("gAlbedoMetallic", gAlbedoMetallic);
    shader->setTexture("gNormalRoughness", gNormalRoughness);
}




void DeferredRendering::Render(Scene const* scene) const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Bind_G_Buffer(DeferredLightingShader);

    // IBL Textures
    DeferredLightingShader->Bind();
    glActiveTexture(GL_TEXTURE20);  // why 20? hard coded
    glBindTexture(GL_TEXTURE_CUBE_MAP, IBL.GetIrradianceMap());

    DeferredLightingShader->setInt("IrradianceMap", 20); 
    
    glActiveTexture(GL_TEXTURE21);  // why 21? hard coded
    glBindTexture(GL_TEXTURE_CUBE_MAP, IBL.GetPrefilterMap());

    DeferredLightingShader->setInt("PrefilterMap", 21);
    DeferredLightingShader->unBind();
    DeferredLightingShader->setTexture("BRDFIntegration", IBL.GetBRDFIntegration());

    DeferredLightingShader->setInt("enableAmbient", enableSSAO);


    const std::vector<Light* > lights = scene->getLightSources();

    for (unsigned int i = 0; i < lights.size(); i++)
    {
        DeferredLightingShader->setVec3("lightPositions[" + std::to_string(i) + "]", lights[i]->getPos());
        DeferredLightingShader->setVec3("lightColors[" + std::to_string(i) + "]", lights[i]->getIntensity());
    }

    DeferredLightingShader->setVec3("camPos", camera.getCameraPos());

    Quad().Draw(*DeferredLightingShader);
}




void DeferredRendering::BindShadowMap(const Shadow& shadow) const
{
    // bind light matrix
    DeferredLightingShader->setMat4("lightProjection", shadow.GetProjection());
    DeferredLightingShader->setMat4("lightView", shadow.GetLightView());

    // bind shadow map
    DeferredLightingShader->setTexture("shadowMap", shadow.GetShadowMap());
}


void DeferredRendering::BindVolumetricLight(const VolumetricLight& vl) const
{
    DeferredLightingShader->setTexture(
        "volumetricLightTexture", vl.GetVolumetricLight());
}


void DeferredRendering::BindSSAO(unsigned int SSAO)
{
    DeferredLightingShader->setTexture("SSAO", SSAO);
}