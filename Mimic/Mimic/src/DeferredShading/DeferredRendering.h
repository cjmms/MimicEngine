#pragma once

//#include "../Core/Shader.h"


class Scene;
class Shader;

class DeferredRendering
{

private:
	/*
	* G-Buffer has 3 Color Buffers
	* Layout:
	*        |  Albedo.r  |  Albedo.g  |  Albedo.b  |  Metallic  |		Unsigned Byte
	*		 | Position.x | Position.y | Position.z |    Empty   |		FLOAT 16 Bit
	*		 |  Normal.x  |  Normal.y  |  Normal.z  |  Roughness |		FLOAT 16 Bit
	*/
	void init_G_Buffer(unsigned int width, unsigned int height);

	Shader* DeferredLightingShader;
	Shader* Fill_G_BufferShader;

	unsigned int gBuffer, gPosition, gNormalRoughness, gAlbedoMetallic;


public:
	DeferredRendering(unsigned int width, unsigned int height);

	void Fill_G_Buffer(Scene const* scene) const;
	void Bind_G_Buffer(Shader* shader) const;
	void Render(Scene const* scene) const;

	inline Shader* GetDeferredShader() const { return DeferredLightingShader; }
};