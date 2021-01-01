#include "Light.h"
#include "Camera.h"

extern Camera camera;

Light::Light(glm::vec3 position, glm::vec3 intensity)
	:position(position), intensity(intensity)
{
	// disable material loading
	lightSphere = new Model("res/objects/sphere/sphere.obj");
}


Light::~Light()
{
	delete lightSphere;
}


void Light::Draw(Shader& shader) const
{
	shader.Bind();

	glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
	model = glm::scale(model, glm::vec3(0.5));
	shader.setMat4("model", model);

	shader.setMat4("projection", camera.getProjectionMatrix());
	shader.setMat4("view", camera.getViewMatrix());

	lightSphere->Draw(shader);

	shader.unBind();
}