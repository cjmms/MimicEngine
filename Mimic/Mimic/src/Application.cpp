#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include <stb_image.h>
#include "Model.h"
#include "Camera.h"
#include "Light.h"


Camera camera;


void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    camera.setCameraKey(window);
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.zoomIn((float)yoffset);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    camera.updateCameraDirection((float)xpos, (float)ypos);
}








int main(void)
{
    GLFWwindow* window;

  
    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1200, 860, "Not a mimic!", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

   
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK)
        std::cout << "GLEW init error" << std::endl;



    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    //Shader shader("res/Shaders/model_loading.shader");
    Shader shader("res/Shaders/basic.shader");
    //shader.Bind();

    Light light(glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(150.0f, 150.0f, 150.0f));

    /*
    glm::vec3 lightPositions[] = {
        glm::vec3(0.0f, 0.0f, 10.0f),
        glm::vec3(0.0f, 20.0f, -10.0f)
    };
    glm::vec3 lightColors[] = {
        glm::vec3(150.0f, 150.0f, 150.0f),
        glm::vec3(150.0f, 150.0f, 150.0f)
    };


        shader.setVec3("lightPositions[0]", lightPositions[0]);
        shader.setVec3("lightColors[0]", lightColors[0]);

        shader.setVec3("lightPositions[1]", lightPositions[1]);
        shader.setVec3("lightColors[1]", lightColors[1]);
    

    // build and compile shaders
    // -------------------------
    //Shader shader("res/Shaders/model_loading.shader");

    // load models
    // -----------
    //Model ourModel("res/objects/backpack/backpack.obj");
    Model ourModel("res/objects/sphere/sphere.obj", false);
    //Model ourModel("res/objects/lion/lion.obj");


    // view/projection transformations


    // render the loaded model 
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(0.4));
    model = glm::translate(model, glm::vec3(0.0, 13.0, 0.0));
    shader.setMat4("model", model);
    shader.setVec3("camPos", camera.getCameraPos());

    */
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        //ourModel.Draw(shader);

        //shader.setMat4("projection", camera.getProjectionMatrix());
        //shader.setMat4("view", camera.getViewMatrix());

        light.Draw(shader);

        camera.cameraUpdateFrameTime();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}