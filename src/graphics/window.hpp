#pragma once
#include <string>
#include <GLFW/glfw3.h>
#include <functional>
#include <vulkan/vulkan.h> // Only for VkExtent2D

namespace graphics
{
class Window
{
public:
    bool open;

    Window();
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;


    void Update();
    void Clear();
    void Close();

    bool IsOpen() const { return !glfwWindowShouldClose(window); }
    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }
    VkExtent2D GetExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }
    GLFWwindow* GetWindow() { return window; }
    bool WindowResized() { return frameBufferResized; }
    void ResetWindowResizedFlag() { frameBufferResized = false; }


    // TODO: Make callbacks work properly
    static void WindowRefreshCallback(GLFWwindow *window);
    static void FrameResizeCallback(GLFWwindow *window, int width, int height);

    static void SetOnRefreshCallback(std::function<void()> callback) { onRefreshCallback = callback; }

private:
    void Init(uint32_t width, uint32_t height, const std::string& title);
    
    uint32_t width;
    uint32_t height;
    bool frameBufferResized = false;

    static std::function<void()> onRefreshCallback;

    std::string name;
    GLFWwindow *window;

    friend class Graphics;
};
}