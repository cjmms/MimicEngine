#include "Shadow.h"
#include "../ResourceManager.h"

#include "../Scene.h"
#include <iostream>




Shadow::Shadow(glm::mat4 View, glm::mat4 Projection, int width, int height)
    :View(View), Projection(Projection),
    ShadowMapShader("res/Shaders/Shadow/DepthMap.shader"),
    MSMShader("res/Shaders/Shadow/MomentMap.shader"),
    Fbo(width, height)
{
    depthBufferFBO = new FBO_Depth(width, height);

    /* 
    glGenFramebuffers(1, &MomentShadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, MomentShadowMapFBO);


    // position
    glGenTextures(1, &Moments);
    glBindTexture(GL_TEXTURE_2D, Moments);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Moments, 0);


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
    */
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

    depthBufferFBO->Unbind();
}


void Shadow::CalculateMSM(Scene const* scene)
{
    // fill depth buffer
    CalculateShadowMap(scene);

    Fbo.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Quad().Draw(MSMShader, GetShadowMap());

    Fbo.Unbind();
}