#include "Shadow.h"
#include "../ResourceManager.h"

#include "../Scene.h"
#include <iostream>




Shadow::Shadow(glm::mat4 View, glm::mat4 Projection, int width, int height)
    :View(View), Projection(Projection),
    ShadowMapShader("res/Shaders/Shadow/DepthMap.shader"),
    MSMShader("res/Shaders/Shadow/MomentMap.shader"),
    VSMShader("res/Shaders/Shadow/VSM.shader")
{
    depthBufferFBO = new FBO_Depth(width, height);
    Fbo = new FBO_Color(width, height);

    SetupVSM(width, height);
}



void Shadow::CalculateShadowMap(Scene const* scene)
{
    depthBufferFBO->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ShadowMapShader.setMat4("view", View);
    ShadowMapShader.setMat4("projection", Projection);

    for (auto obj : scene->getObjects())
    {
        ShadowMapShader.setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(ShadowMapShader);
    }
    scene->RenderPlane(&ShadowMapShader);

    depthBufferFBO->Unbind();
}


void Shadow::CalculateMSM(Scene const* scene)
{
    /*
    // fill depth buffer
    CalculateShadowMap(scene);

    Fbo->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Quad().Draw(MSMShader, GetShadowMap());

    Fbo->Unbind();
    */

    Fbo->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    MSMShader.setMat4("view", View);
    MSMShader.setMat4("projection", Projection);

    for (auto obj : scene->getObjects())
    {
        MSMShader.setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(MSMShader);
    }
    scene->RenderPlane(&MSMShader);

    Fbo->Unbind();
}




void Shadow::ComputeVSM(Scene const* scene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, VSM_FBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    VSMShader.setMat4("view", View);
    VSMShader.setMat4("projection", Projection);

    for (auto obj : scene->getObjects())
    {
        VSMShader.setMat4("model", obj->getModelMatrix());
        obj->getModel()->Draw(VSMShader);
    }
    scene->RenderPlane(&VSMShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}




void Shadow::SetupVSM(unsigned int shadowMapRes_w, unsigned int shadowMapRes_h)
{
    glGenFramebuffers(1, &VSM_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, VSM_FBO);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glGenTextures(1, &VSMDepthTexture);
    glBindTexture(GL_TEXTURE_2D, VSMDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowMapRes_w, shadowMapRes_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Clamp to border to fix over-sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, VSMDepthTexture, 0);

    // Colour texture for Variance Shadow Mapping (VSM)
    if (VSMColorTexture) {
        glDeleteTextures(1, &VSMColorTexture);
    }
    glGenTextures(1, &VSMColorTexture);
    glBindTexture(GL_TEXTURE_2D, VSMColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, shadowMapRes_w, shadowMapRes_h, 0, GL_RGBA, GL_FLOAT, nullptr);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Hardware linear filtering gives us soft shadows for free!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Clamp to border to fix over-sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_caps.MaxAnisotropy); // Anisotropic filtering for sharper angles
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, VSMColorTexture, 0);
    

    // check if framebuffer created successfullly
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: VSM_FBO is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}