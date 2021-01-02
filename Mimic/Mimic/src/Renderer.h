#pragma once
#include "Core/Shader.h"
#include "Scene.h"
#include "Core/FBO.h"


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
	Shader *PBR_Forward_Shader;
	Shader *lightShader;
	Shader *Fill_G_Buffer;
	Shader *DeferredShader;

	Shader *ShadowMapShader;

	unsigned int gBuffer;

	FBO_Depth *depthBufferFBO;
	
	void BindLightSources(Shader* shader, Scene const* scene) const;

	/*
	* G-Buffer has 3 Color Buffers
	* Layout: 
	*        |  Albedo.r  |  Albedo.g  |  Albedo.b  |  Metallic  |		Unsigned Byte
	*		 | Position.x | Position.y | Position.z |    Empty   |		FLOAT 16 Bit
	*		 |  Normal.x  |  Normal.y  |  Normal.z  |  Roughness |		FLOAT 16 Bit
	*/
	void init_G_Buffer(unsigned int width, unsigned int height);

	void Draw(Shader *shader, Scene const* scene) const;

	void DeferredRender(Scene const* scene) const;

	inline bool isDeferred() const { return DEFERRED == type; }

	void passDepthMap(Shader* shader);

public:
	Renderer(RenderingType type);

	~Renderer();
	
	void Render(Scene const* scene);

	// shader is always fixed for light source
	// the purpose of rendering light is for testing
	void RenderLightSources(Scene const* scene) const;

	void setDepthMap(Scene const* scene);
};

