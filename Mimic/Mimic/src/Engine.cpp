#include "Engine.h"
#include "Core/Camera.h"
#include "UI_Manager.h"

#include "Core/Shader.h"
#include "Core/Model.h"
#include "Scene.h"



Camera camera;
UI_Manager UI_Mgr;


void Engine::init()
{
	UI_Mgr.init();

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
}

void Engine::close()
{
	UI_Mgr.close();
}

void Engine::run()
{
    Scene lightScene;
    lightScene.addLightSource(glm::vec3(-5.0f, 15.0f, 10.0f), glm::vec3(150.0f, 150.0f, 150.0f));
    lightScene.addLightSource(glm::vec3(40.0f, 30.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    lightScene.addLightSource(glm::vec3(100.0f, 20.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));

    Shader shader("res/Shaders/model_loading.shader");

    // load models
    // -----------
    //Model backpack("res/objects/backpack/backpack.obj");
    Model sponza("res/objects/sponza/sponza.obj");

    lightScene.BindLightSources(shader);

    shader.Bind();
    // render the loaded model 
    glm::mat4 model = glm::mat4(1.0);
    model = glm::scale(model, glm::vec3(0.1));
    shader.setMat4("model", model);
    shader.setVec3("camPos", camera.getCameraPos());


    // debug mode?
    //Shader quadShader("res/Shaders/Quad.shader");

    
    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.cameraUpdateFrameTime();

        shader.Bind();
        shader.setMat4("projection", camera.getProjectionMatrix());
        shader.setMat4("view", camera.getViewMatrix());

        sponza.Draw(shader);

        lightScene.RenderLightSources();


        //Quad().Draw(quadShader);
    }
}