#pragma once
#include "graphics/api/resources/material.hpp"
#include "graphics/backend/buffer.hpp"
#include "graphics_pipeline.hpp"
#include "graphics/api/shader_layout.hpp"
#include "descriptors.hpp"

namespace graphics
{
// TODO: Handle better, need a material manager
class ShaderBuffer // Represents a UBO struct
{
public:
    ShaderBuffer(BufferLayout layout, const uint8_t *data);
    ShaderBuffer(const Material *material);

    VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }

private:
    const Material *material = nullptr; // TODO: Remove material dependency
    Buffer buffer; // TODO: Replace with index into larger buffer, this won't scale

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};

}