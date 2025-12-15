#pragma once
#include <memory>

#include "backend/vulkan_backend.hpp"
#include "rendering/renderer.hpp"
#include "resources/descriptor_set.hpp"
#include "resources/descriptor_buffer.hpp"
#include "resources/pipeline_manager.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"

namespace graphics
{
class Graphics;

struct CameraUbo
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 invView;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProj;
};
enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};
struct Light
{
    glm::vec3 position{};
    int type = DIRECTIONAL;
    glm::vec3 color{};
    float intensity = 1.0f;
};
struct GlobalUbo
{
    glm::vec3 ambient{};
    int numLights = 0;
    Light lights[128]{};
};
// Global graphics data can go here
// Maybe singleton?
class GraphicsData
{
public:
    ~GraphicsData();
    internal::VulkanBackend &GetBackend() { return backend; }
    Window &GetWindow() { return window; }
    GLFWwindow *GetGLFWWindow() { return window.GetWindow(); }

    GlobalUbo globalUbo{};
    CameraUbo cameraUbo{};

    // std::unique_ptr<DescriptorPool> globalDescriptorPool;
    // std::unique_ptr<DescriptorPool> cameraDescriptorPool;
    // std::unique_ptr<DescriptorPool> materialDescriptorPool;

    std::unique_ptr<DescriptorBuffer> globalDescriptorBuffer; // Ooh shiny new thing
    std::unique_ptr<DescriptorBuffer> frameBuffer; // For per-frame data like camera UBOs
    std::unique_ptr<DescriptorBuffer> materialBuffer; // Material data
    std::unique_ptr<DescriptorBuffer> objectBuffer; // Object data like model matrices
    // Global pool
    // Global descriptor sets
    // Push constants
    // Default textures, shaders, materials
private:
// Declaration order matters here

    Window window;
    internal::VulkanBackend backend;
    Renderer renderer{backend.GetDevice(), window};
    PipelineManager pipelineManager{backend.GetDevice()};

    friend class Graphics;
};

extern std::unique_ptr<GraphicsData> graphicsData;
// Failed attempt lol
// // Construct in data/bss after main starts rather than before
// GraphicsData& graphicsData = []() -> GraphicsData& {
//     static GraphicsData instance;
//     return instance;
// }();
} // namespace graphics