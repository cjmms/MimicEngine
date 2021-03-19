#include "VolumetricLight.h"
#include "../Core/Camera.h"
#include "../ResourceManager.h"
#include "../Shadow/Shadow.h"


extern Camera camera;

VolumetricLight::VolumetricLight(unsigned int width, unsigned int height)
	:BilateralUpShader("res/Shaders/BilateralUp.shader"),
	VolumetricLightShader("res/Shaders/VolumetricLighting.shader"),
	LightingFBO(width, height), HalfResFBO(width / 2, height / 2),
	width(width), height(height)
{
}


void VolumetricLight::Compute(const Shadow& shadow, unsigned int gPosition)
{
    // half resolution ray marching
    HalfResFBO.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind light matrix
    VolumetricLightShader.setMat4("lightProjection", shadow.GetProjection());
    VolumetricLightShader.setMat4("lightView", shadow.GetLightView());

    // bind shadow map
    VolumetricLightShader.setTexture("shadowMap", shadow.GetShadowMap());

    // bind gPosition
    VolumetricLightShader.setTexture("gPosition", gPosition);

    VolumetricLightShader.setVec3("camPos", camera.getCameraPos());

    Quad().Draw(VolumetricLightShader);

    HalfResFBO.Unbind();


    LightingFBO.Bind();
    // Bilateral Upsampling to resolution
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    BilateralUpShader.setTexture("volumetricLightTexture", HalfResFBO.getColorAttachment());
    BilateralUpShader.setTexture("gPosition", gPosition);

    BilateralUpShader.setInt("BilateralSwitch", 1);

    Quad().Draw(BilateralUpShader);
    LightingFBO.Unbind();
}