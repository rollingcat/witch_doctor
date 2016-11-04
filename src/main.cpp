
#include <iostream>

#include "VulkanRenderer.h"

int main(int argc, char *argv[]) {
    std::cout << "VooDoo: Begin" << std::endl;

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Voodoo by Witch Doctor", nullptr, nullptr);

    VulkanRenderer renderer(window);

    while (!glfwWindowShouldClose(window)) {
        renderer.render();

        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "VooDoo: End" << std::endl;
    return 0;
}
