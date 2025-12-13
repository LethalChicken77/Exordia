#include "graphics_data.hpp"
namespace graphics
{
// GraphicsData graphicsData{};
std::unique_ptr<GraphicsData> graphicsData;

GraphicsData::~GraphicsData()
{
    backend.WaitForDevice();
    globalDescriptorBuffer.reset();
    frameBuffer.reset();
    materialBuffer.reset();
    objectBuffer.reset();
}
} // namespace graphics