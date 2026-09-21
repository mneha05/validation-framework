#include <GLFW/glfw3.h>
#include <iostream>

int main() {
    if (!glfwInit()) {
        std::cerr << "failed to initialize GLFW\n";
        return 1;
    }
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(64, 64, "probe", nullptr, nullptr);
    if (!window) {
        std::cerr << "failed to create OpenGL context\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    std::cout << "OpenGL vendor: " << glGetString(GL_VENDOR) << "\n";
    std::cout << "OpenGL renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << "\n";
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
