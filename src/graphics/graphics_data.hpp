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
    const internal::VulkanBackend &GetBackend() { return backend; }
    Window &GetWindow() { return window; }
    // Global pool
    // Global descriptor sets
    // Push constants
    // Default textures, shaders, materials
private:
    internal::VulkanBackend backend;
    Window window{800, 600, "VEngine"}; // TODO: Unhardcode values
    friend class Graphics;
};
extern GraphicsData graphicsData;
} // namespace graphics