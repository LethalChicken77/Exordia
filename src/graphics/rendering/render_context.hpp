#pragma once

#include "graphics/backend/vulkan_include.h"

namespace graphics
{

struct RenderContext
{
    uint32_t frameIndex;
    uint32_t imageIndex;
    VkCommandBuffer commandBuffer;
};

} // namespace graphics