#pragma once
#include <memory>

#include "backend/vulkan_backend.hpp"
#include "rendering/renderer.hpp"
#include "resources/descriptors.hpp"
#include "resources/pipeline_manager.hpp"
#include "resources/shader_buffer.hpp"
#include "resources/registries/mesh_registry.hpp"
#include "resources/registries/material_registry.hpp"
#include "resources/registries/pipeline_registry.hpp"
#include "resources/graphics_material.hpp"


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
private:
// Declaration order matters here
    Window window;
    internal::VulkanBackend backend;
    Renderer renderer{backend.GetDevice(), window};

public:
    ~GraphicsData();
    inline internal::VulkanBackend &GetBackend() { return backend; }
    inline internal::Device &GetDevice() { return backend.GetDevice(); }
    inline const internal::PhysicalDevice &GetPhysicalDevice() const { return backend.GetPhysicalDevice(); }
    inline const VkPhysicalDeviceProperties2 &GetDeviceProperties() const { return backend.GetPhysicalDevice().GetProperties(); }
    Window &GetWindow() { return window; }
    GLFWwindow *GetGLFWWindow() { return window.GetWindow(); }

    GlobalUbo globalUboData{};
    CameraUbo cameraUboData{};

    MeshRegistry meshRegistry{};
    MaterialRegistry materialRegistry{};
    PipelineRegistry pipelineRegistry{backend.GetDevice()};

    std::unique_ptr<DescriptorPool> globalDescriptorPool{};
    std::unique_ptr<DescriptorPool> cameraDescriptorPool{};
    std::unique_ptr<DescriptorPool> materialDescriptorPool{};
    std::unique_ptr<DescriptorPool> imguiDescriptorPool{};

    std::unique_ptr<DescriptorSetLayout> globalSetLayout{};
    std::unique_ptr<DescriptorSetLayout> cameraSetLayout{};

    std::unique_ptr<Buffer> globalUBO{};
    std::vector<Buffer> cameraUBOs{};

    VkDescriptorSet globalDescriptorSet = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> cameraDescriptorSets{};
    // Global pool
    // Push constants
    // Default textures, shaders, materials
private:
    // PipelineManager pipelineManager{backend.GetDevice()};

    friend class Graphics;
};

extern std::unique_ptr<GraphicsData> graphicsData;

} // namespace graphics