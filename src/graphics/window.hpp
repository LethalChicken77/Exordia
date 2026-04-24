#pragma once
#include <string>
#include "graphics/backend/vulkan_include.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>

namespace graphics::internal{class VulkanBackend;}
namespace graphics
{
enum class WindowMode
{
    Windowed,
    Borderless,
    Fullscreen
};

class Window
{
public:
    bool open;

    Window(uint32_t width, uint32_t height, const std::string& title);
    Window(vk::Instance& instance, uint32_t width, uint32_t height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void Update();
    void Clear();
    void Close();

    bool IsOpen() const { return !glfwWindowShouldClose(window); }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    float GetAspectRatio() const { return (float)m_width / (float)m_height; }
    vk::Extent2D GetExtent() { return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}; }
    GLFWwindow* GetWindow() { return window; }
    vk::SurfaceKHR &GetSurface() { return surface; }
    bool WindowResized() { return m_frameBufferResized; }
    void ResetWindowResizedFlag() { m_frameBufferResized = false; }

    void SetWindowMode(WindowMode mode);
    void ToggleFullscreenBorderless();
    void ToggleFullscreen();
    // TODO: Events
    static void WindowRefreshCallback(GLFWwindow *window);
    static void FrameResizeCallback(GLFWwindow *window, int width, int height);
    static void WindowMoveCallback(GLFWwindow *window, int posX, int posY);
    static void WindowMaximizeCallback(GLFWwindow *window, int val);
    static void WindowMinimizeCallback(GLFWwindow *window, int val);
    static void WindowFocusCallback(GLFWwindow *window, int val);

    static void SetOnRefreshCallback(std::function<void()> callback) { onRefreshCallback = callback; }

private:
    vk::Instance& m_instance;
    WindowMode m_mode;
    uint32_t m_posX;
    uint32_t m_posY;
    uint32_t m_width;
    uint32_t m_height;

    // Windowed mode info
    uint32_t m_wPosX;
    uint32_t m_wPosY;
    uint32_t m_wWidth;
    uint32_t m_wHeight;

    bool m_maximized = false;
    bool m_minimized = false;
    bool m_focused = false;
    
    bool m_frameBufferResized = false;

    static std::function<void()> onRefreshCallback;

    std::string name;
    GLFWwindow *window;
    vk::SurfaceKHR surface;

    void createSurface();
    void setTitleBarColor();
    void setIcons();

    void updateWindowedVals();
};
}