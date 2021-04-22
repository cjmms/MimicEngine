#include "Engine.h"
#include "Core/Camera.h"
#include "UI_Manager.h"
#include "Scene.h"
#include <iostream>
#include "Renderer.h"
#include "Core/Model.h"
#include "Core/Shader.h"
#include "Core//FBO.h"
#include "ResourceManager.h"
#include <sstream>


#define SHADOW_MAP_DEBUG 0




/*
* DONE: BRDF
* DONE: Camera
* DONE: Deferred Rendering Pipeline
* DONE: Forward Rendering Pipeline
* DONE: Shadow Mapping
* DONE: Point Light
* DONE: Model Loading
* DONE: Texture Loading
* DONE: Volumetric lighting
* DONE: VSM
* DONE: MSM
* 
* TODO: imGUI
* TODO: SSAO
* TODO: bloom
* TODO: Spot Light
* TODO: HDR with adaptive exposure
* TODO: light volume
* 
* TODO: instancing
* TODO: SSR
* TODO: Depth of Field with Bokeh
* TODO: Voxel based global illumination
* TODO: Camera and object motion blur
* TODO: Precomputed Atmospheric Scattering
*/

Camera camera;
UI_Manager UI_Mgr;


void Engine::init()
{
	UI_Mgr.init();

    glEnable(GL_DEPTH_TEST);
    
    // init scene
    scene = new Scene();
    scene->addLightSource(glm::vec3(-20.0f, 35.0f, 10.0f), glm::vec3(550.0f, 550.0f, 550.0f));
    //scene->addLightSource(glm::vec3(40.0f, 30.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    //scene->addLightSource(glm::vec3(40.0f, 10.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    //scene->addLightSource(glm::vec3(20.0f, 30.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    //scene->addLightSource(glm::vec3(20.0f, 30.0f, -10.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    //scene->addLightSource(glm::vec3(110.0f, 20.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    //scene->addLightSource(glm::vec3(-70.0f, 70.0f, 0.0f), glm::vec3(350.0f, 350.0f, 350.0f));

    //scene->addObjects("res/objects/sponza/sponza.obj", glm::vec3(0.1));

    //scene->addObjects("res/objects/lion/lion.obj", glm::vec3(5.0));
    //scene->InitTextures("res/objects/lion/");

    //scene->addObjects("res/objects/Pokemon/Pokemon.obj", glm::vec3(0.1f));
    //scene->InitTextures("res/objects/Pokemon/");

    //scene->addObjects("res/objects/Knife/Knife.obj", glm::vec3(10.0));
    //scene->InitTextures("res/objects/Knife/");

    //scene->addObjects("res/objects/Axe/Axe.obj", glm::vec3(1.0));
    //scene->InitTextures("res/objects/Axe/");

    scene->addObjects("res/objects/backpack/backpack.obj", glm::vec3(3.0));

    /*
    // dragon
    for (int i = 0; i < 16; ++i) {
        std::stringstream ss;
        if (i < 10) ss << "res/objects/Dragon/models/Mesh00" << i << ".obj";
        else  ss << "res/objects/Dragon/models/Mesh0" << i << ".obj";
        scene->addObjects(ss.str().c_str(), glm::vec3(1.0f));
    }*/
}

void Engine::close()
{
	UI_Mgr.close();
}



void Engine::run()
{
    Renderer renderer(scene);

    //UI_Mgr.enableCursor();
    //camera.disable();

    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();

        //camera.Print();

        camera.cameraUpdateFrameTime();

        UI_Mgr.NewUIFrame();      

        renderer.Render(scene);
        renderer.RenderLightSources(scene);

        UI_Mgr.RenderUI();
    }
}