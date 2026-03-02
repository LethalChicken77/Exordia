#pragma once
#include <array>
#include "graphics/backend/vulkan_include.h"

namespace graphics::internal
{
    inline constexpr std::array<const char *, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    inline constexpr std::array<const char *, 6> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_8BIT_STORAGE_EXTENSION_NAME,
        VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
        VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
        // VK_EXT_FILTER_CUBIC_EXTENSION_NAME, 
        // VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME 
    };

    struct Features
    {
        vk::StructureChain<
            vk::PhysicalDeviceFeatures2, // Make sure this stays first
            vk::PhysicalDeviceVulkan11Features,
            vk::PhysicalDeviceVulkan12Features,
            vk::PhysicalDeviceVulkan13Features,
            vk::PhysicalDeviceVulkan14Features> featureChain;
        
        Features();
        Features(const Features&) = default;
        // Features(const Features&&) = delete;

        bool operator==(const Features &other) const;
    };
    extern Features features;
        
} // namespace graphics::internal