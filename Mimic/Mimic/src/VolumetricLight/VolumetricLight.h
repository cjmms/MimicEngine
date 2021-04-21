#pragma once

#include "../Core/Shader.h"
#include "../Core/FBO.h"

class Shadow;

class VolumetricLight
{
private:
	Shader BilateralUpShader;
	Shader VolumetricLightShader;

	//FBO_Color LightingFBO;
	//FBO_Color HalfResFBO;

	unsigned int width, height;

	int Steps;
	float G_SCATTERING;
	bool enableDithering;
	bool BilateralSwitch;


public:
	VolumetricLight(unsigned int width, unsigned int height);
	void Compute(const Shadow& shadow, unsigned int gPosition);
	inline unsigned int GetVolumetricLight() const { return LightingFBO.getColorAttachment(); }

	FBO_Color LightingFBO;
	FBO_Color HalfResFBO;

	void UI();
};

