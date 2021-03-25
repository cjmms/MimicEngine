#include "IBL.h"
#include "../ResourceManager.h"
#include "../Core/Camera.h"
#include "../Scene.h"
#include <iostream>

extern Camera camera;



IBL::IBL(const char* address, unsigned int res)
	:res(res)
{
	// used to render HDR Equirectangular map to a cube
	Equirectangular2CubemapShader = new Shader("res/Shaders/IBL/equirectangular.shader");
	CubemapShader = new Shader("res/Shaders/IBL/Cubemap.shader");
	HDR_Env = ResourceManager::loadHDRTexture(address);

	InitCubemapTexture(res);
	InitCubemapFBO(res);
	
}





void IBL::RenderEquirectangular2Cube(Scene const* scene) const
{
	Equirectangular2CubemapShader->setMat4("view", camera.getViewMatrix());
	Equirectangular2CubemapShader->setMat4("projection", camera.getProjectionMatrix());

	Equirectangular2CubemapShader->setTexture("equirectangularMap", HDR_Env);
	scene->RenderCube(Equirectangular2CubemapShader);
}



void IBL::InitCubemapFBO(unsigned int res)
{
	unsigned int captureRBO;
	glGenFramebuffers(1, &CubemapFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, CubemapFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, res, res);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



void IBL::InitCubemapTexture(unsigned int res)
{
	glGenTextures(1, &EnvCubemapTex);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemapTex);
	for (unsigned int i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, res, res, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}




void IBL::RenderCubemap(Scene const* scene)
{
	glm::mat4 captureViews[] =
	{
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// convert HDR equirectangular environment map to cubemap equivalent
	Equirectangular2CubemapShader->setMat4("projection", glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f));
	Equirectangular2CubemapShader->setTexture("equirectangularMap", HDR_Env);

	glDepthFunc(GL_LEQUAL);
	glViewport(0, 0, res, res); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, CubemapFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		Equirectangular2CubemapShader->setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, EnvCubemapTex, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		scene->RenderCube(Equirectangular2CubemapShader);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void IBL::RenderSkybox(Scene const* scene) const
{
	CubemapShader->setMat4("projection", camera.getProjectionMatrix());
	CubemapShader->setMat4("view", camera.getViewMatrix());

	CubemapShader->Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, EnvCubemapTex);
	CubemapShader->setInt("equirectangularMap", 0);
	CubemapShader->unBind();

	scene->RenderCube(CubemapShader);
}