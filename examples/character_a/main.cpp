#include <GLFW/glfw3.h>
#include <stdexcept>


int main() {
    if(glfwInit() == GLFW_FALSE) {
        std::runtime_error("failed to initialize GLFW");
    }

    GLFWwindow* window = glfwCreateWindow(500, 500, "character_a", nullptr, nullptr);
    if(window == nullptr) {
        std::runtime_error("failed to create GLFWwindow");
    }


    while(glfwWindowShouldClose(window) == GLFW_FALSE) {

        glfwSwapBuffers(window);
        glfwWaitEvents();
    }

    glfwTerminate();
}