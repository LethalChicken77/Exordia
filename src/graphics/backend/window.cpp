#include <iostream>
#include "window.hpp"
#include "utils/debug.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

namespace graphics
{
Window::Window(uint32_t _width, uint32_t _height, const std::string& title)
    : width(_width), height(_height), name(title)
{
    init();
}

Window::~Window()
{
    Close();
}

void Window::init()
{
    // Initialize GLFW
    if(!glfwInit())
    {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Set GLFW to not create an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(640, 480, name.c_str(), NULL, NULL);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FrameResizeCallback);
    // glfwSetWindowRefreshCallback(window, windowRefreshCallback);

    if (!window) 
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    open = true;
    return;
}

void Window::Clear()
{
    // Clear the window
}

void Window::Close()
{
    if(surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(VK_NULL_HANDLE, surface, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::CreateWindowSurface(VkInstance instance)
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create window surface!");
    }
}

std::function<void()> Window::onRefreshCallback = nullptr;

void Window::WindowRefreshCallback(GLFWwindow *_window)
{
    Window *windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(_window));

    int width, height;
    glfwGetFramebufferSize(windowPtr->window, &width, &height);

    windowPtr->frameBufferResized = true;
    windowPtr->width = width;
    windowPtr->height = height;

    if(windowPtr->onRefreshCallback)
    {
        windowPtr->onRefreshCallback();
    }
}

void Window::FrameResizeCallback(GLFWwindow *_window, int width, int height)
{
    Window *windowPtr = reinterpret_cast<Window*>(glfwGetWindowUserPointer(_window));
    windowPtr->frameBufferResized = true;
    windowPtr->width = width;
    windowPtr->height = height;
}

} // namespace graphics