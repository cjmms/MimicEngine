#pragma once

#include <vector>
#include "../Core/Shader.h"

class SSAO
{
private:
	std::vector<glm::vec3> ssaoKernel;
	std::vector<glm::vec3> ssaoNoise;

	// 4 X 4 texture by default
	unsigned int NoiseTexture;
	unsigned int KernelSize;
	unsigned int NumberOfNoise;

	inline float lerp(float a, float b, float f) const { return a + f * (b - a); }

	// disribute samples closer to origin
	inline float WeightToSample(float i) const { lerp(0.1f, 1.0f, powf( i / KernelSize, 2)); }

	// add random noise into ssaoNoise
	void FillNoise();

public:
	void ComputeSampleKernel();
	void ComputeNoiseTexture();

};

