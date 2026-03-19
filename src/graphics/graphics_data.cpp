#include "graphics_data.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
namespace graphics
{
// GraphicsData graphicsData{};
std::unique_ptr<GraphicsData> graphicsData;

GraphicsData::~GraphicsData()
{
    backend.WaitForDevice();

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    pipelineManager.DestroyPipelines();
    meshRegistry.Reset();

    cameraDescriptorPool.reset();
    globalDescriptorPool.reset();
    materialDescriptorPool.reset();
    imguiDescriptorPool.reset();
    cameraSetLayout.reset();
    globalSetLayout.reset();
    testMaterial.reset();

    globalUBO.reset();
    cameraUBOs.clear();

    backend.Cleanup();
}
} // namespace graphics