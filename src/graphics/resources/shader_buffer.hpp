#pragma once
#include "buffer.hpp"
#include "graphics_pipeline.hpp"
#include "graphics/resources/layouts/shader_layout.hpp"
#include "descriptors.hpp"

namespace graphics
{
// TODO: Handle better, need a material manager
class ShaderBuffer // Represents a UBO struct
{
public:
    ShaderBuffer(BufferLayout layout, const uint8_t *data);

    VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }

private:
    Buffer buffer; // TODO: Replace with index into larger buffer, this won't scale

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

}