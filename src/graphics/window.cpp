#include <iostream>
#include "window.hpp"
#include "graphics/graphics_data.hpp"
#include "utils/debug.hpp"
#include "utils/console.hpp"
#include <stdexcept>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <dwmapi.h>
// #pragma comment(lib, "Dwmapi.lib")
#endif
#include "stb_image.h"

namespace graphics
{
Window::Window(uint32_t _width, uint32_t _height, const std::string& title)
    : Window(graphicsData->GetBackend().GetInstance(), _width, _height, title)
{}
Window::Window(vk::Instance& instance, uint32_t _width, uint32_t _height, const std::string& title)
    : m_instance(instance),
    width(_width), height(_height),
    name(title)
{
    Console::log("Initializing window: " + title, "Window");

    // Set GLFW to not create an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(640, 480, name.c_str(), NULL, NULL);
    setTitleBarColor();
    setIcons();

    // GLFWmonitor* primary = glfwGetPrimaryMonitor();
    // const GLFWvidmode* mode = glfwGetVideoMode(primary);
    // glfwSetWindowMonitor(window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, FrameResizeCallback);
    // glfwSetWindowRefreshCallback(window, windowRefreshCallback);

    if (!window) 
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    open = true;

    createSurface();
}

Window::~Window()
{
    if(surface != VK_NULL_HANDLE)
        m_instance.destroySurfaceKHR(surface, nullptr);
    Close();
}


void Window::createSurface()
{
    VkSurfaceKHR tempSurface{};
    VkResult result = glfwCreateWindowSurface(m_instance, window, nullptr, &tempSurface);
    surface = tempSurface;
    if(result != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create window surface: " + Debug::VkResultToString(result));
    }
}

void Window::Clear()
{
    // Clear the window :D
}

void Window::Close()
{
    glfwDestroyWindow(window);
    glfwTerminate();
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


#ifdef _WIN32
COLORREF titleBarColor = RGB(0x19, 0x15, 0x14);
COLORREF borderColor = RGB(0x19, 0x15, 0x14);
void Window::setTitleBarColor() 
{
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd) {
        // std::cout << "Setting title color" << std::endl;
        // Use DwmSetWindowAttribute to set the title bar color
        const DWORD titleBarColorAttribute = DWMWA_CAPTION_COLOR;
        const DWORD borderColorAttribute = DWMWA_BORDER_COLOR;
        DwmSetWindowAttribute(hwnd, titleBarColorAttribute, &titleBarColor, sizeof(titleBarColor));
        DwmSetWindowAttribute(hwnd, borderColorAttribute, &borderColor, sizeof(borderColor));
    }
}
#endif

void Window::setIcons()
{
    stbi_set_flip_vertically_on_load(false); // Window icons are not flipped
    // Load small icon
    int widthSmall, heightSmall, channelsSmall;
    unsigned char* pixelsSmall = stbi_load("internal/images/icon_48.png", &widthSmall, &heightSmall, &channelsSmall, 4);  // Load as RGBA
    // std::cout << "Width: " << widthSmall << " Height: " << heightSmall << std::endl;
    if (!pixelsSmall) {
        std::cerr << "Failed to load small icon!" << std::endl;
        stbi_image_free(pixelsSmall);
    }

    // Load large icon
    int widthLarge, heightLarge, channelsLarge;
    unsigned char* pixelsLarge = stbi_load("internal/images/icon_256.png", &widthLarge, &heightLarge, &channelsLarge, 4);  // Load as RGBA
    // std::cout << "Width: " << widthLarge << " Height: " << heightLarge << std::endl;
    if (!pixelsLarge) {
        std::cerr << "Failed to load large icon!" << std::endl;
        stbi_image_free(pixelsSmall);  // Free previously loaded image
        stbi_image_free(pixelsLarge);
    }

    // Create GLFWimage array for icons
    GLFWimage icons[2];
    icons[0].width = widthSmall;
    icons[0].height = heightSmall;
    icons[0].pixels = pixelsSmall;

    icons[1].width = widthLarge;
    icons[1].height = heightLarge;
    icons[1].pixels = pixelsLarge;

    // Set the window icon
    glfwSetWindowIcon(window, 2, icons);

    // Free image memory
    stbi_image_free(pixelsSmall);
    stbi_image_free(pixelsLarge);
}

} // namespace graphics