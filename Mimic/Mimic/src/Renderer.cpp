#include "Renderer.h"
#include "Core/Camera.h"

extern Camera camera;

Renderer::Renderer(bool isDeferred)
	:isDeferred(isDeferred), lightShader(new Shader("res/Shaders/basic.shader"))
{
	glEnable(GL_DEPTH_TEST);

	if (!isDeferred)
		PBR_Forward_Shader = new Shader("res/Shaders/model_loading.shader");
}



Renderer::~Renderer()
{}



void Renderer::Render(Scene const* scene) const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	PBR_Forward_Shader->Bind();

	// pass light source
	BindLightSources(PBR_Forward_Shader, scene);

	// pass projection and view matrix
	PBR_Forward_Shader->setMat4("projection", camera.getProjectionMatrix());
	PBR_Forward_Shader->setMat4("view", camera.getViewMatrix());

	// pass camera position
	PBR_Forward_Shader->setVec3("camPos", camera.getCameraPos());

	for (auto obj : scene->getObjects()) obj->Draw(*PBR_Forward_Shader);

	PBR_Forward_Shader->unBind();
}


void Renderer::RenderLightSources(Scene const* scene) const
{
	//glDisable(GL_DEPTH_TEST);

	// This is a bad design, since Light* can still be changed
	// The right way should be passing a pair of iterators instead ( Maybe? )
	const std::vector<Light* > lights = scene->getLightSources();
	for (auto light : lights) light->Draw(*lightShader);

	//glEnable(GL_DEPTH_TEST);
}






void Renderer::BindLightSources(Shader* shader, Scene const* scene) const
{
	const std::vector<Light* > lights = scene->getLightSources();

	for (unsigned int i = 0; i < lights.size(); i++)
	{
		shader->setVec3("lightPositions[" + std::to_string(i) + "]", lights[i]->getPos());
		shader->setVec3("lightColors[" + std::to_string(i) + "]", lights[i]->getIntensity());
	}
}