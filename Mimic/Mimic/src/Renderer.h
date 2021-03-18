#pragma once
#include "Core/Shader.h"
#include "Scene.h"
#include "Core/FBO.h"
#include "DeferredShading/DeferredRendering.h"


/*
* Purpose of Renderer: render object base on input, hide all details of how rendering works
* 
* The renderer will render base on the infromation inside scene object
* There are 2 types of renderer: deferred renderer, forward renderer.
* 
* 1. Renderer will init and run differerntly base on its type
* 
* 2. Type can't be changed
* 
* 3. Shaders will be inside the renderer, bind/unbind in side the renderer, pass uniform inside renderer
* 
* 4. Other part of the engine should not access shader at all
*/



enum RenderingType
{
	DEFERRED,
	FORWARD
};



class Renderer
{
private:
	RenderingType type;
	Shader *lightShader;


	Shader *ShadowMapShader;

	// some shaders for debuging purposes
	Shader* DepthQuadShader;
	Shader* BilateralUpShader;
	Shader* VolumetricLightShader;
	Shader* ColorQuadShader;

	DeferredRendering DeferredRenderer;


	FBO_Depth *depthBufferFBO;
	FBO_Color* LightingFBO;
	FBO_Color* HalfResFBO;

	bool debugMode;


	void DeferredRender(Scene const* scene) const;

	void bindShadowMap(Shader* shader) const;

	void VolumetricLight(FBO_Color* fbo) const;

	inline bool isDeferred() const { return DEFERRED == type; }


	// Fill the depth map
	// Draw a scene by using view and projection matrix provided
	// the Shadow map will store inside depth attachment of depthBufferFBO
	void RenderShadowMap(glm::mat4 view, glm::mat4 projection, Scene const* scene);



	// some helper functions for debugging

	// Visualize depth buffer
	// render depth buffer in a quad
	// This function will not check if depthAttachment is a depth buffer or not
	void VisualizeDepthBuffer(unsigned int depthAttachment) const;


public:
	Renderer(RenderingType type, bool debugMode = true);

	~Renderer();
	
	void Render(Scene const* scene);

	// shader is always fixed for light source
	// the purpose of rendering light is for testing
	void RenderLightSources(Scene const* scene) const;
};

