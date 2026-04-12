#pragma once

#include "graphics/backend/vulkan_include.h"

namespace graphics
{

struct FrameContext
{
    uint32_t frameIndex = ~0u; // Intended to crash if the default value is used
    uint32_t imageIndex = ~0u;
    vk::CommandBuffer commandBuffer = nullptr;
};

struct PassInfo
{
    vk::ImageView colorView;
    vk::ImageView depthView;
    vk::Extent2D extent;
    vk::ClearValue clearColor;
};

} // namespace graphics