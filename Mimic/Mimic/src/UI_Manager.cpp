#include "UI_Manager.h"
#include <iostream>
#include <stb_image.h>


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






bool UI_Manager::init()
{
    /* Initialize the library */
    if (!glfwInit())
    {
        std::cout << "GLFW failed to init." <<  std::endl;
        return false;
    }

    // default screen size
    window_width = 1200;
    window_height = 860;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(window_width, window_height, "Not a mimic!", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        std::cout << "GLFW window failed to init." << std::endl;
        return false;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW init error" << std::endl;
        return false;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    disableCursor();

    return true;
}



void UI_Manager::close()
{
    glfwTerminate();
}


void UI_Manager::update()
{
    processInput(window);

   // camera.cameraUpdateFrameTime();

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
}



void UI_Manager::disableCursor()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}



void UI_Manager::enableCursor()
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}


void UI_Manager::setScreenSize(unsigned int window_width, unsigned int window_height)
{
    if (window_height <= 0 || window_width <= 0)
    {
        std::cout << "Invalid screen size." << std::endl;
        return;
    }

    this->window_width = window_width;
    this->window_height = window_height;
}



