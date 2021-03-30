#include "Shadow.h"
#include "../ResourceManager.h"

#include "../Scene.h"
#include <iostream>




Shadow::Shadow(glm::mat4 View, glm::mat4 Projection, int width, int height)
    :View(View), Projection(Projection),
    ShadowMapShader("res/Shaders/Shadow/DepthMap.shader"),
    VSMShader("res/Shaders/Shadow/VSM.shader")
{
    Setup(width, height);
}






void Shadow::Compute(Scene const* scene)
{
    glBindFramebuffer(GL_FRAMEBUFFER, Shadow_FBO);
    glEnable(GL_DEPTH_TEST);
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




void Shadow::Setup(unsigned int shadowMapRes_w, unsigned int shadowMapRes_h)
{
    glGenFramebuffers(1, &Shadow_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Shadow_FBO);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glGenTextures(1, &DepthTexture);
    glBindTexture(GL_TEXTURE_2D, DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowMapRes_w, shadowMapRes_h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Clamp to border to fix over-sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);


    glGenTextures(1, &ColorTexture);
    glBindTexture(GL_TEXTURE_2D, ColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, shadowMapRes_w, shadowMapRes_h, 0, GL_RGBA, GL_FLOAT, nullptr);

    // Hardware linear filtering gives us soft shadows for free!
    // My hardware is stupid, it only gives me bugs
    //glGenerateMipmap(GL_TEXTURE_2D);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); // Clamp to border to fix over-sampling
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ColorTexture, 0);
    

    // check if framebuffer created successfullly
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: VSM_FBO is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}