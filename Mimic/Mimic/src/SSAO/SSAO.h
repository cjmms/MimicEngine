#pragma once

#include <vector>
#include "../Core/Shader.h"

class SSAO
{
private:
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;
	Shader SSAOshader;

	// 4 X 4 texture by default
	unsigned int NoiseTexture;
	unsigned int KernelSize;
	unsigned int NoiseTextureLength;

	unsigned int ssaoFBO;
	unsigned int ssaoColorBuffer;

	inline float lerp(float a, float b, float f) const { return a + f * (b - a); }

	// disribute samples closer to origin
	inline float WeightToSample(float i) const { return lerp(0.1f, 1.0f, powf( i / KernelSize, 2)); }

	// add random noise into ssaoNoise
	void FillNoise();

	// 2D quad for SSAO
	void FrameBufferSetup(unsigned int width, unsigned int height);

	void SendKernelSamplesToShader();

public:
	inline unsigned int GetNoiseTex() const { return NoiseTexture; }
	inline unsigned int GetSSAO() const { return ssaoColorBuffer; }

	SSAO(unsigned int KernelSize, unsigned int NoiseTextureLength);

	void ComputeSampleKernel();
	void ComputeNoiseTexture();

	void RenderSSAO(unsigned int gPosition, unsigned int gNormal);

};

