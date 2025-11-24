#pragma once

#include "camera.hpp"

#include <vulkan/vulkan.h>

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