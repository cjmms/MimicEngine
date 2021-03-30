#pragma once

#include "../Core/Shader.h"
#include <glm/glm.hpp>
#include "../Core/FBO.h"

class Scene;


class Shadow
{
private:
	Shader ShadowMapShader;
	Shader VSMShader;

	const glm::mat4 View;
	const glm::mat4 Projection;

	unsigned int Shadow_FBO, DepthTexture, ColorTexture;


public:
	Shadow(glm::mat4 View, glm::mat4 Projection, int width, int height);

	inline glm::mat4 GetLightView() const { return View; }
	inline glm::mat4 GetProjection() const { return Projection; }

	// returns a handle to a texture
	inline unsigned int GetShadowMap() const { return ColorTexture; }

	void Compute(Scene const* scene);

	void Setup(unsigned int shadowMapRes_w, unsigned int shadowMapRes_h);
};

