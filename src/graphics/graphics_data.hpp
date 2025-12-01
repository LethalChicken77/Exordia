#pragma once
#include "backend/vulkan_backend.hpp"

namespace graphics
{
class Graphics;
// Global graphics data can go here
// Maybe singleton?
class GraphicsData
{
public:
    internal::VulkanBackend &GetBackend() { return backend; }
    Window &GetWindow() { return window; }
    GLFWwindow *GetGLFWWindow() { return window.GetWindow(); }
    // Global pool
    // Global descriptor sets
    // Push constants
    // Default textures, shaders, materials
private:
    Window window;
    internal::VulkanBackend backend;
    friend class Graphics;
};

// Construct in data/bss after main starts rather than before
inline GraphicsData& graphicsData = []() -> GraphicsData& {
    static GraphicsData instance;
    return instance;
}();
} // namespace graphics