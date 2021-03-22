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