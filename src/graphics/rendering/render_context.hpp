#pragma once

#include "graphics/backend/vulkan_include.h"

namespace graphics
{
    struct RenderContext
    {
        uint32_t frameIndex;
        double frameTime;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet globalDescriptorSet;
        VkDescriptorSet cameraDescriptorSet;
    };
} // namespace graphics