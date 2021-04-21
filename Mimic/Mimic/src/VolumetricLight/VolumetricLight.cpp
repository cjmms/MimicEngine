#include "VolumetricLight.h"
#include "../Core/Camera.h"
#include "../ResourceManager.h"
#include "../Shadow/Shadow.h"

#include "../imgui/imgui.h"

//#include "imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_internal.h"


extern Camera camera;

VolumetricLight::VolumetricLight(unsigned int width, unsigned int height)
	:BilateralUpShader("res/Shaders/VolumetricLight/BilateralUp.shader"),
	VolumetricLightShader("res/Shaders/VolumetricLight/VolumetricLighting.shader"),
	LightingFBO(width, height), HalfResFBO(width / 2, height / 2),
	width(width), height(height), Steps(10), enableDithering(true),
    BilateralSwitch(true), G_SCATTERING(0.45f)
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
    VolumetricLightShader.setTexture("shadowMap", shadow.GetRangeShadowMap());

    // bind gPosition
    VolumetricLightShader.setTexture("gPosition", gPosition);

    VolumetricLightShader.setVec3("camPos", camera.getCameraPos());

    VolumetricLightShader.setInt("MarchingSteps", Steps);
    VolumetricLightShader.setInt("enableDithering", enableDithering);
    VolumetricLightShader.setFloat("G_SCATTERING", G_SCATTERING);

    Quad().Draw(VolumetricLightShader);

    HalfResFBO.Unbind();


    LightingFBO.Bind();
    // Bilateral Upsampling to resolution
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    BilateralUpShader.setTexture("volumetricLightTexture", HalfResFBO.getColorAttachment());
    BilateralUpShader.setTexture("gPosition", gPosition);

    BilateralUpShader.setInt("BilateralSwitch", BilateralSwitch);

    Quad().Draw(BilateralUpShader);
    LightingFBO.Unbind();
}




void VolumetricLight::UI()
{
    ImGui::SliderInt("Marching steps", &Steps, 1, 100);
    ImGui::Checkbox("Enable Dithering", &enableDithering);
    ImGui::Checkbox("Enable Bilateral Upsampling", &BilateralSwitch);
    ImGui::SliderFloat("Asymmetry factor", &G_SCATTERING, -1, 1);
}