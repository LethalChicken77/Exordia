#pragma once
#include "core/resources/material.hpp"
#include "graphics/backend/buffer.hpp"
#include "graphics_pipeline.hpp"
#include "graphics/api/layout.hpp"
#include "descriptor_set.hpp"

namespace graphics
{
// TODO: Handle better, need a material manager
class ShaderBuffer // Represents a UBO struct
{
public:
    ShaderBuffer(ShaderLayout layout);
    ShaderBuffer(const core::Material *material);

    VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }

private:
    const core::Material *material; // TODO: Remove material dependency
    Buffer buffer; // TODO: Replace with index into larger buffer, this won't scale

    VkDescriptorSet descriptorSet;
};

}