#pragma once

#include "DeferredShading/DeferredRendering.h"
#include "VolumetricLight/VolumetricLight.h"
#include "Shadow/Shadow.h"
#include "IBL/IBL.h"
#include "SSAO/SSAO.h"

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





class Renderer
{
private:
	Shader *lightShader;

	// some shaders for debuging purposes
	Shader* DepthQuadShader;
	Shader* ColorQuadShader;
	Shader* ForwardShader;
	Shader* GaussianBlurShader;

	FBO_Color PingBufferFBO;
	FBO_Color PongBufferFBO;

	DeferredRendering DeferredRenderer;
	Shadow* shadow;
	VolumetricLight VolumetricLight;


	bool debugMode;



	// Visualize depth buffer
	// render depth buffer in a quad
	// This function will not check if depthAttachment is a depth buffer or not
	void VisualizeDepthBuffer(unsigned int depthAttachment) const;

	unsigned int GaussianBlur(unsigned int texture, int level) const;

	void RenderUI();

	void ShadowUI();


public:
	Renderer(Scene const* scene);


	~Renderer();
	
	void Render(Scene const* scene);

	void ForwardRendering(Scene const* scene);

	// shader is always fixed for light source
	// the purpose of rendering light is for testing
	void RenderLightSources(Scene const* scene) const;

	
};

