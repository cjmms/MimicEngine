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
    VolumetricLightShader.setTexture("shadowMap", shadow.GetRangeShadowMap());

    // bind gPosition
    VolumetricLightShader.setTexture("gPosition", gPosition);

    VolumetricLightShader.setVec3("camPos", camera.getCameraPos());

    VolumetricLightShader.setFloat("g", g);
    VolumetricLightShader.setInt("sampleN", sampleN);

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


void VolumetricLight::RenderUI()
{
    //ImGui::Text("ShadowMap Gaussian Blur Pass.");
    ImGui::SliderInt("sample number", &sampleN, 2, 300);
    ImGui::SliderFloat("scattering factor", &g, 0.01, 1.0);

    //ImGui::RadioButton("Standard Shadow Map", &ShadowMapType, 0);
    //ImGui::RadioButton("Variance Shadow Map", &ShadowMapType, 1);
    //ImGui::RadioButton("Moment Shadow Map", &ShadowMapType, 2);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}