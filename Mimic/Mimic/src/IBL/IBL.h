#pragma once
#include <string>
class Scene;
class Shader;

class IBL
{
private:
	// Equirectangular HDR texture
	unsigned int HDR_Env;

	unsigned int CubemapFBO, CubemapRBO;

	// cubemap texture
	unsigned int EnvCubemapTex;

	unsigned int res;

	unsigned int cubeVAO;

	// Cubemap, irradiance Map
	unsigned int IrradianceMapTex;

	// render Equirectangular HDR texture to a cube
	Shader* Equirectangular2CubemapShader;
	Shader* CubemapShader;
	Shader* IrradianceShader;

	// res: resolution of each face of cubemap
	void InitCubemapFBO(unsigned int res);
	void InitCubemapTexture(unsigned int res);

	// since irradiance map is calculated based on uniformly average surrounding radiance 
	// Irradiance map texture does not need high resolution
	void InitIrradianceMapTexture(unsigned int lowRes);

	// generate a cubemap from HDR_Env
	void Equirectangular2Cubemap();


	void initCube();
	void RenderCube(Shader* shader) const;

	// using Equirectangular to render 6 faces of cubemap
	void RenderCubemap(Shader* shader, unsigned int res, unsigned int outputCubemapTex);

	void RenderIrradianceMap(unsigned int lowRes);

public:
	IBL(const char* , unsigned int res);

	IBL(std::string& address, unsigned int res) : IBL(address.c_str(), res) {}


	void RenderEquirectangular2Cube() const;

	void RenderSkybox() const;
};

