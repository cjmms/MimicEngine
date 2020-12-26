#include "Light.h"
#include "Camera.h"

extern Camera camera;

Light::Light(glm::vec3 position, glm::vec3 intensity)
	:position(position), intensity(intensity)
{
	// disable material loading
	lightSphere = new Model("res/objects/sphere/sphere.obj", false);
}


Light::~Light()
{
	delete lightSphere;
}


void Light::Draw(Shader& shader)
{
	shader.Bind();

	glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1));
	model = glm::translate(model, position);
	shader.setMat4("model", model);

	shader.setMat4("projection", camera.getProjectionMatrix());
	shader.setMat4("view", camera.getViewMatrix());

	lightSphere->Draw(shader);

	shader.unBind();
}