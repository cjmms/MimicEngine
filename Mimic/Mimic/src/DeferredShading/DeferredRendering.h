#pragma once
#include "../IBL/IBL.h"

class Scene;
class Shader;
class Shadow;
class VolumetricLight;

class DeferredRendering
{

private:
	/*
	* G-Buffer has 3 Color Buffers
	* Layout:
	*        |  Albedo.r  |  Albedo.g  |  Albedo.b  |  Metallic  |		Unsigned Byte
	*		 | Position.x | Position.y | Position.z | Visibility |		FLOAT 16 Bit
	*		 |  Normal.x  |  Normal.y  |  Normal.z  |  Roughness |		FLOAT 16 Bit
	*/
	void init_G_Buffer(unsigned int width, unsigned int height);

	Shader* DeferredLightingShader;
	Shader* Fill_G_BufferShader;

	unsigned int gBuffer, gPosition, gNormalRoughness, gAlbedoMetallic;

	IBL IBL;

public:
	DeferredRendering(unsigned int width, unsigned int height);

	void Fill_G_Buffer(Scene const* scene) const;
	void Bind_G_Buffer(Shader* shader) const;
	void Render(Scene const* scene) const;

	inline Shader* GetDeferredShader() const { return DeferredLightingShader; }
	inline unsigned int Get_G_Position() const { return gPosition; }
	inline unsigned int Get_G_NormalRoughness() const { return gNormalRoughness; }
	inline unsigned int Get_G_AlbedoMetallic() const { return gAlbedoMetallic; }

	void BindShadowMap(const Shadow& shadow) const;
	void BindVolumetricLight(const VolumetricLight& vl) const;
	inline Shader* GetFillBufferShader() const { return Fill_G_BufferShader; }
};