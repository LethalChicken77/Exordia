#include "shader_buffer.hpp"
#include "graphics/graphics_data.hpp"

namespace graphics
{

ShaderBuffer::ShaderBuffer(BufferLayout layout, const uint8_t *data)
    : buffer(
        layout.GetSize(),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        graphicsData->GetBackend().GetPhysicalDevice().GetProperties().properties.limits.minUniformBufferOffsetAlignment
    )
{
    buffer.Map();
    buffer.WriteData((void*)data, layout.GetSize(), 0);
    buffer.Unmap();

}

} // namespace graphics