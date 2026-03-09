#include "shader_buffer.hpp"
#include "graphics/graphics_data.hpp"
#include "pipeline_manager.hpp"

namespace graphics
{

ShaderBuffer::ShaderBuffer(const core::Material *material) 
    : buffer(
        material->GetBufferSize(),
        1,
        VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        graphicsData->GetBackend().GetPhysicalDevice().GetProperties().properties.limits.minUniformBufferOffsetAlignment
    )
{
    const std::vector<uint8_t> &data = material->GetData();
    buffer.Map();
    buffer.WriteData((void*)data.data(), data.size(), 0);
    buffer.Unmap();
}

} // namespace graphics