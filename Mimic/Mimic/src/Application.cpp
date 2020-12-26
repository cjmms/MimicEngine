#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Core/VertexBuffer.h"
#include "Core/IndexBuffer.h"
#include "Core/Shader.h"
#include <stb_image.h>
#include "Core/Model.h"
#include "Core/Camera.h"
#include "Core/Light.h"
#include "UI_Manager.h"




Camera camera;
UI_Manager UI_Mgr;

int main(void)
{
    UI_Mgr.init();


    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    Light light1(glm::vec3(10.0f, 0.0f, -10.0f), glm::vec3(150.0f, 150.0f, 150.0f));
    Light light2(glm::vec3(0.0f, 10.0f, -10.0f), glm::vec3(150.0f, 150.0f, 150.0f));

    Shader lightShader("res/Shaders/basic.shader");
    Shader shader("res/Shaders/model_loading.shader");
    shader.Bind();

    shader.setVec3("lightPositions[0]", light1.getPos());
    shader.setVec3("lightColors[0]", light1.getIntensity());

    shader.setVec3("lightPositions[1]", light2.getPos());
    shader.setVec3("lightColors[1]", light2.getIntensity());
    
    // load models
    // -----------
    //Model ourModel("res/objects/backpack/backpack.obj");
    Model lion("res/objects/lion/lion.obj");


    // render the loaded model 
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0));
    shader.setMat4("model", model);


    shader.setVec3("camPos", camera.getCameraPos());

    
    while(!UI_Mgr.windowClosed())
    { 
        UI_Mgr.update();


        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.cameraUpdateFrameTime();


        shader.Bind();
        shader.setMat4("projection", camera.getProjectionMatrix());
        shader.setMat4("view", camera.getViewMatrix());
        lion.Draw(shader);

        light1.Draw(lightShader);
        light2.Draw(lightShader);

    }


    UI_Mgr.close();
    return 0;
}