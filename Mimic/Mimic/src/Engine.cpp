#include "Engine.h"
#include "Core/Camera.h"
#include "UI_Manager.h"

#include "Core/Shader.h"
#include "Core/Model.h"
#include "Scene.h"




/*
* TODO: shadow mapping
* TODO: deferred shading
* TODO: imGUI
* TODO: Volumetric lighting
* TODO: SSAO
* TODO: soft shadow
*/

Camera camera;
UI_Manager UI_Mgr;


void Engine::init()
{
	UI_Mgr.init();

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);


    // init scene
    scene = new Scene();
    scene->addLightSource(glm::vec3(-5.0f, 15.0f, 10.0f), glm::vec3(550.0f, 550.0f, 550.0f));
    scene->addLightSource(glm::vec3(40.0f, 30.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    scene->addLightSource(glm::vec3(110.0f, 20.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    scene->addLightSource(glm::vec3(-40.0f, 100.0f, 0.0f), glm::vec3(350.0f, 350.0f, 350.0f));

    scene->addObjects("res/objects/sponza/sponza.obj", glm::vec3(0.1));
    //Model backpack("res/objects/backpack/backpack.obj");
}

void Engine::close()
{
	UI_Mgr.close();
}



void Engine::run()
{
    Shader shader("res/Shaders/model_loading.shader");

    //Shader quadShader("res/Shaders/Quad.shader");

    
    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.cameraUpdateFrameTime();

        scene->RenderObjects(shader);
        scene->RenderLightSources();

        //Quad().Draw(quadShader);
    }
}