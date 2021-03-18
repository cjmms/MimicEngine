#include "Shadow.h"


void Shadow::BindShadowMap(Shader* shader) const
{
    // bind light matrix
    shader->setMat4("lightProjection", Projection);
    shader->setMat4("lightView", View);

    // bind shadow map
    shader->setTexture("shadowMap", depthBufferFBO->getDepthAttachment());
}


Shadow::Shadow(glm::mat4 View, glm::mat4 Projection, int width, int height)
    :View(View), Projection(Projection),
    ShadowMapShader("res/Shaders/DepthMap.shader") 
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