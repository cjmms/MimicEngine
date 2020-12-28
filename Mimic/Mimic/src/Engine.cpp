#include "Engine.h"
#include "Core/Camera.h"
#include "UI_Manager.h"

#include "Core/Shader.h"
#include "Scene.h"
#include "ResourceManager.h"

#include <iostream>



/*
* TODO: screen resize
* TODO: FBO
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

    Shader quadShader("res/Shaders/Quad.shader");

    // test only
    unsigned int texture = ResourceManager::loadTexture("res/hanon.jpg", false);

    //UI_Mgr.enableCursor();


    //------------------------------------------------------------
    unsigned int FBO, RBO, colorAttachment;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    // generate FBO color attachment, bind to current FBO
    glGenTextures(1, &colorAttachment);
    glBindTexture(GL_TEXTURE_2D, colorAttachment);  // bind texture

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);        // unbind texture


    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorAttachment, 0);

    // generate FBO depth, stencil attachment(24 bits, 8 bits), bind to current FBO
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    // check if framebuffer created successfullly
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //------------------------------------------------------------------------------



    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();


        camera.cameraUpdateFrameTime();

        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->RenderObjects(shader);
        scene->RenderLightSources();


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Quad().Draw(quadShader, colorAttachment);
    }
}