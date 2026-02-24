#include "graphics_data.hpp"
namespace graphics
{
// GraphicsData graphicsData{};
std::unique_ptr<GraphicsData> graphicsData;

GraphicsData::~GraphicsData()
{
    backend.WaitForDevice();

    pipelineManager.DestroyPipelines();
    cameraDescriptorPool.reset();
    globalDescriptorPool.reset();
    cameraSetLayout.reset();
    globalSetLayout.reset();

}
} // namespace graphics