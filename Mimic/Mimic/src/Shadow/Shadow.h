#pragma once

#include "../Core/Shader.h"
#include <glm/glm.hpp>
#include "../Scene.h"
#include "../Core/FBO.h"


class Shadow
{
private:
	Shader ShadowMapShader;
	FBO_Depth* depthBufferFBO;
	glm::mat4 View;
	glm::mat4 Projection;


public:
	Shadow(glm::mat4 View, glm::mat4 Projection, int width, int height);

	inline glm::mat4 GetLightView() const { return View; }
	inline glm::mat4 GetProjection() const { return Projection; }

	// returns a handle to a texture
	inline unsigned int GetShadowMap() const { return depthBufferFBO->getDepthAttachment(); }
	void CalculateShadowMap(Scene const* scene);
};

