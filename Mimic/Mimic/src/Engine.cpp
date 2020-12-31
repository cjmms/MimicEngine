#include "Engine.h"
#include "Core/Camera.h"
#include "UI_Manager.h"

#include "Core/Shader.h"
#include "Scene.h"
#include "ResourceManager.h"

#include <iostream>
#include "Core/FBO.h"


#define SHADOW_MAP_DEBUG 0


/*
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
        glm::vec3(-40.0f, 80.0f, 0.0f),
        glm::vec3(30.0f, 60.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // width / height is 1.0, front plane: 0.1f, back plane 125.0f
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);


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
    Shader shader("res/Shaders/model_loading.shader");
    Shader Fill_G_Buffer("res/Shaders/FillG-Buffer.shader");
    Shader ColorQuadShader("res/Shaders/ColorQuad.shader");
    Shader DeferredShader("res/Shaders/DeferredPBR.shader");


    //-------------------------------------------------------------------------------------------
    // init G-buffer
    unsigned int width = UI_Mgr.getScreenWidth();
    unsigned int height = UI_Mgr.getScreenHeight();


    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPositionDepth, gNormalRoughness, gAlbedoMetallic;

    // Albedo + Metallic
    glGenTextures(1, &gAlbedoMetallic);
    glBindTexture(GL_TEXTURE_2D, gAlbedoMetallic);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedoMetallic, 0);


    // Position + Depth
    glGenTextures(1, &gPositionDepth);
    glBindTexture(GL_TEXTURE_2D, gPositionDepth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gPositionDepth, 0);


    // Normal + Roughness
    glGenTextures(1, &gNormalRoughness);
    glBindTexture(GL_TEXTURE_2D, gNormalRoughness);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gNormalRoughness, 0);

    //-------------------------------------------------------------------------------------------


    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int G_Buffer[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, G_Buffer);

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // G-Buffer created
    //--------------------------------------------------------------------------


    // bind G buffer to texture unit
    DeferredShader.Bind();
    DeferredShader.setInt("gPositionDepth", 6);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, gPositionDepth);

    DeferredShader.setInt("gAlbedoMetallic", 7);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, gAlbedoMetallic);

    DeferredShader.setInt("gNormalRoughness", 8);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, gNormalRoughness);

    DeferredShader.setVec3("camPos", camera.getCameraPos());

    DeferredShader.setVec3("lightPositions[0]", glm::vec3(-5.0f, 15.0f, 10.0f));
    DeferredShader.setVec3("lightColors[0]", glm::vec3(550.0f, 550.0f, 550.0f));

    DeferredShader.unBind();





    

    
    glm::mat4 lightView = glm::lookAt(
        glm::vec3(-60.0f, 70.0f, 0.0f),
        glm::vec3(30.0f, 60.0f, 55.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    // width / height is 1.0, front plane: 0.1f, back plane 125.0f
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 325.0f);

    FBO_Depth depthBufferFBO(UI_Mgr.getScreenWidth(), UI_Mgr.getScreenHeight());
    //FBO_Depth depthBufferFBO(860, 860);

    Shader depthQuadShader("res/Shaders/DepthQuad.shader");
    Shader ShadowMapShader("res/Shaders/DepthMap.shader");

    shader.Bind();
    shader.setMat4("lightView", lightView);
    shader.setMat4("lightProjection", lightProjection);
    
    shader.setInt("shadowMap", 7);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, depthBufferFBO.getDepthAttachment());

    shader.unBind();
    

    //UI_Mgr.enableCursor();

    while (!UI_Mgr.windowClosed())
    {
        UI_Mgr.update();

        camera.cameraUpdateFrameTime();


       /* std::cout << "position: " << camera.getCameraPos().x << ", " << 
            camera.getCameraPos().y << ", " << 
            camera.getCameraPos().z << std::endl;
       */

        //depthBufferFBO.Bind();
        //glClear( GL_DEPTH_BUFFER_BIT);
        //scene->RenderShadowMap(lightView, lightProjection, ShadowMapShader );


        //depthBufferFBO.Unbind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //if (SHADOW_MAP_DEBUG) {
          //  Quad().Draw(depthQuadShader, depthBufferFBO.getDepthAttachment());
        //}
        //else {
            scene->RenderObjects(shader);
            scene->RenderLightSources();
        //}

/*
        // Fill G buffer
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        scene->RenderObjects(Fill_G_Buffer);
        //scene->RenderLightSources();

        // unbind G buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //Quad().Draw(ColorQuadShader, gAlbedoMetallic);    // works
        //Quad().Draw(ColorQuadShader, gPositionDepth);     // fails
        Quad().Draw(DeferredShader, gNormalRoughness);   // works
        */
    }
}