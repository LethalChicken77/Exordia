#include "graphics_data.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
namespace graphics
{
// GraphicsData graphicsData{};
std::unique_ptr<GraphicsData> graphicsData;

GraphicsData::~GraphicsData()
{
    Console::log("Destroying graphics data", "GraphicsData");
    backend.WaitForDevice();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    meshRegistry.Reset();
    materialRegistry.Reset();
    pipelineRegistry.Cleanup();
    textureRegistry.Reset();

    cameraDescriptorPool.reset();
    globalDescriptorPool.reset();
    materialDescriptorPool.reset();
    imguiDescriptorPool.reset();
    cameraSetLayout.reset();
    globalSetLayout.reset();
    
    globalUBO.reset();
    cameraUBOs.clear();
    
    renderer.reset();
    window.reset();
    backend.WaitForDevice();
    backend.Cleanup(this);
    Console::log("Destroyed graphics data", "GraphicsData");
}
} // namespace graphics