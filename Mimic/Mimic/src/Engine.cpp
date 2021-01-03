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
* 
* TODO: imGUI
* TODO: Volumetric lighting
* TODO: SSAO
* TODO: bloom
* TODO: Spot Light
* TODO: HDR with adaptive exposure
* TODO: Cascaded Shadow Mapping
* TODO: soft shadow
* TODO: light volume
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
    scene->addLightSource(glm::vec3(-5.0f, 15.0f, 10.0f), glm::vec3(550.0f, 550.0f, 550.0f));
    scene->addLightSource(glm::vec3(40.0f, 30.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    scene->addLightSource(glm::vec3(110.0f, 20.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    scene->addLightSource(glm::vec3(-60.0f, 70.0f, 0.0f), glm::vec3(350.0f, 350.0f, 350.0f));

    scene->addObjects("res/objects/sponza/sponza.obj", glm::vec3(0.1));
    //Model backpack("res/objects/backpack/backpack.obj");
    
}

void Engine::close()
{
	UI_Mgr.close();
}



void Engine::run()
{
    Renderer renderer(DEFERRED);
    //Renderer renderer(FORWARD);

    //renderer.setDepthMap(scene);
    /*
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-60.0f, 70.0f, 0.0f),
        glm::vec3(30.0f, 60.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // width / height is 1.0, front plane: 0.1f, back plane 125.0f
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);
    */

    //UI_Mgr.enableCursor();
    
    //renderer.setDepthMap(scene);

    //Model bag("res/objects/backpack/backpack.obj");

    //Shader shader("res/Shaders/DepthMap.shader");
    //Shader depthquad("res/Shaders/DepthQuad.shader");
    //Shader colorquad("res/Shaders/ColorQuad.shader");

    //FBO_Depth fbo(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());

    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();

        camera.cameraUpdateFrameTime();
       // glClear( GL_DEPTH_BUFFER_BIT);
       /* std::cout << "position: " << camera.getCameraPos().x << ", " << 
            camera.getCameraPos().y << ", " << 
            camera.getCameraPos().z << std::endl;
       */


        //fbo.Bind();
        // resolution of shadow map
        //glViewport(0, 0, 860, 860);
        /*
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Bind();
        shader.setMat4("view", camera.getViewMatrix());
        shader.setMat4("projection", camera.getProjectionMatrix());
        glm::mat4 model(1.0);
        shader.setMat4("model", model);

        bag.Draw(shader);
        fbo.Unbind();


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //quad.Bind();
        Quad::Quad().Draw(depthquad, fbo.getDepthAttachment());
        */

        renderer.Render(scene);
        renderer.RenderLightSources(scene);
    }
}