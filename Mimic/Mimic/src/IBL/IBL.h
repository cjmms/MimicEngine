#pragma once
#include <string>
class Scene;
class Shader;

class IBL
{
private:
	// Equirectangular HDR texture
	unsigned int HDR_Env;

	unsigned int CubemapFBO;

	// cubemap texture
	unsigned int EnvCubemapTex;

	unsigned int res;

	// render Equirectangular HDR texture to a cube
	Shader* Equirectangular2CubemapShader;
	Shader* CubemapShader;

	// res: resolution of each face of cubemap
	void InitCubemapFBO(unsigned int res);
	void InitCubemapTexture(unsigned int res);


public:
	IBL(const char* , unsigned int res);

	IBL(std::string& address, unsigned int res) : IBL(address.c_str(), res) {}


	//void Setup();

	void RenderEquirectangular2Cube(Scene const* scene) const;

	void RenderSkybox(Scene const* scene) const;
	void RenderCubemap(Scene const* scene);
};

