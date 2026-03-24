#include "shader_buffer.hpp"
#include "graphics/graphics_data.hpp"
#include "pipeline_manager.hpp"

namespace graphics
{

ShaderBuffer::ShaderBuffer(BufferLayout layout, const uint8_t *data)
    : buffer(
        layout.GetSize(),
        1,
        VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        graphicsData->GetBackend().GetPhysicalDevice().GetProperties().properties.limits.minUniformBufferOffsetAlignment
    )
{
    buffer.Map();
    buffer.WriteData((void*)data, layout.GetSize(), 0);
    buffer.Unmap();

}

} // namespace graphics