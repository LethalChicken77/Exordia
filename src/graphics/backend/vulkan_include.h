#pragma once
// #define VK_NO_PROTOTYPES
#include <Volk/volk.h>
// #include <vulkan/vulkan.h>
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 0
#include <vulkan.hpp>
#include <vulkan_enums.hpp>
#include <vk_enum_string_helper.h>

#ifdef DEBUG
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1
#define VMA_LEAK_LOG_FORMAT(format, ...) do { \
        printf((format), __VA_ARGS__); \
        printf("\n"); \
    } while(false)
// #define VMA_DEBUG_LOG_FORMAT(format, ...) do { \
//     printf((format), __VA_ARGS__); \
//     printf("\n"); \
// } while(false)
#endif
// #include <vma/vk_mem_alloc.h>
#include "vk_mem_alloc.hpp"