#include <GLFW/glfw3.h>
#include <cstdint>
#include <stdexcept>
#include <iostream>

#include <vulkan/vulkan.hpp>

#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan_engine.hpp>

int main() {
    if(glfwInit() == GLFW_FALSE) {
        throw std::runtime_error("failed to initialize GLFW");
    }

    GLFWwindow* window = glfwCreateWindow(500, 500, "character_a", nullptr, nullptr);
    if(window == nullptr) {
        throw std::runtime_error("failed to create GLFWwindow");
    }

    {
        VulkanEngine engine { "character_a" };
        engine.initialize();

        if(!engine.isInitialized()) {
            throw std::runtime_error("failed to initialize Vulkan");
        }

        auto& physicalDevice = engine.getPhysicalDevice();

        while(glfwWindowShouldClose(window) == GLFW_FALSE) {

            glfwSwapBuffers(window);
            glfwWaitEvents();
        }
    }

    glfwTerminate();
}