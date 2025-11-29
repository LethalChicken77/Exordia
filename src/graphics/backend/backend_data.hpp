#pragma once
#include <array>
#include <Volk/volk.h>

namespace graphics::internal
{
    inline constexpr std::array<const char *, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    inline constexpr std::array<const char *, 2> deviceExtensions = { // Uncomment extensions to use them
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
        // VK_EXT_FILTER_CUBIC_EXTENSION_NAME, 
        // VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME 
    };
} // namespace graphics::internal