#pragma once

#include "../Core/Shader.h"
#include <glm/glm.hpp>
#include "../Core/FBO.h"

class Scene;


class Shadow
{
private:
	Shader ShadowMapShader;
	Shader MSMShader;

	FBO_Depth* depthBufferFBO;
	FBO_Color Fbo;

	glm::mat4 View;
	glm::mat4 Projection;


public:
	Shadow(glm::mat4 View, glm::mat4 Projection, int width, int height);

	inline glm::mat4 GetLightView() const { return View; }
	inline glm::mat4 GetProjection() const { return Projection; }

	// returns a handle to a texture
	inline unsigned int GetShadowMap() const { return depthBufferFBO->getDepthAttachment(); }
	inline unsigned int GetMSM() const { return Fbo.getColorAttachment(); }

	void CalculateShadowMap(Scene const* scene);
	void CalculateMSM(Scene const* scene);
};

