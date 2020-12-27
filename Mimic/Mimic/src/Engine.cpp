#include "Engine.h"
#include "Core/Camera.h"
#include "UI_Manager.h"


#include "Core/Shader.h"
#include <stb_image.h>
#include "Core/Model.h"
#include "Core/Light.h"
#include "ResourceManager.h"

#include "Core/VertexBuffer.h"
#include "iostream"


Camera camera;
UI_Manager UI_Mgr;


void Engine::init()
{
	UI_Mgr.init();
}

void Engine::close()
{
	UI_Mgr.close();
}

void Engine::run()
{
    
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    /*

    Light light1(glm::vec3(5.0f, 5.0f, -10.0f), glm::vec3(150.0f, 150.0f, 150.0f));

    Shader lightShader("res/Shaders/basic.shader");
    Shader shader("res/Shaders/model_loading.shader");
    shader.Bind();

    shader.setVec3("lightPositions[0]", light1.getPos());
    shader.setVec3("lightColors[0]", light1.getIntensity());


    // load models
    // -----------
    //Model backpack("res/objects/backpack/backpack.obj");
    Model lion("res/objects/lion/lion.obj");


    // render the loaded model 
    shader.setMat4("model", glm::mat4(1.0f));
    shader.setVec3("camPos", camera.getCameraPos());

    */
    Shader quadShader("res/Shaders/Quad.shader");
    Rectangle quad;

    

    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();


        /* Render here */
        glClearColor(0.5, 0.5, 0.5, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //camera.cameraUpdateFrameTime();

        //shader.Bind();
        //shader.setMat4("projection", camera.getProjectionMatrix());
        //shader.setMat4("view", camera.getViewMatrix());
        //lion.Draw(shader);


        //light1.Draw(lightShader);

        quad.Draw(quadShader);
    }
}