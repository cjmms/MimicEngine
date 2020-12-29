#include "Engine.h"
#include "Core/Camera.h"
#include "UI_Manager.h"

#include "Core/Shader.h"
#include "Scene.h"
#include "ResourceManager.h"

#include <iostream>
#include "Core/FBO.h"



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


    // light view and light projection are hard coded for testing volumetric lighting
    // CSM should be used to implement shadow instead rather than basic shadow mapping
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-40.0f, 90.0f, 0.0f),
        glm::vec3(30.0f, 60.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // width / height is 1.0, front plane: 0.1f, back plane 125.0f
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 125.0f);


    // init scene
    scene = new Scene();
    scene->addLightSource(glm::vec3(-5.0f, 15.0f, 10.0f), glm::vec3(550.0f, 550.0f, 550.0f));
    scene->addLightSource(glm::vec3(40.0f, 30.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    scene->addLightSource(glm::vec3(110.0f, 20.0f, -20.0f), glm::vec3(350.0f, 350.0f, 350.0f));
    //scene->addLightSource(glm::vec3(-40.0f, 90.0f, 0.0f), glm::vec3(350.0f, 350.0f, 350.0f));

    scene->addObjects("res/objects/sponza/sponza.obj", glm::vec3(0.1));
    //Model backpack("res/objects/backpack/backpack.obj");
}

void Engine::close()
{
	UI_Mgr.close();
}



void Engine::run()
{
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-40.0f, 90.0f, 0.0f),
        glm::vec3(30.0f, 60.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // width / height is 1.0, front plane: 0.1f, back plane 125.0f
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 125.0f);



    Shader shader("res/Shaders/model_loading.shader");

    Shader depthQuadShader("res/Shaders/DepthQuad.shader");
    Shader ShadowMapShader("res/Shaders/DepthMap.shader");


    FBO_Depth depthBufferFBO(860, 860);

    UI_Mgr.enableCursor();

    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();

        camera.cameraUpdateFrameTime();

        
        depthBufferFBO.Bind();
        glClear( GL_DEPTH_BUFFER_BIT);
        scene->RenderShadowMap(lightView, lightProjection, ShadowMapShader );

        /*
        depthBufferFBO.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->RenderObjects(shader);
        scene->RenderLightSources();
        */

        depthBufferFBO.Unbind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Quad().Draw(depthQuadShader, depthBufferFBO.getDepthAttachment());
        
    }
}