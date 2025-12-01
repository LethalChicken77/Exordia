#pragma once
#include <vulkan/vulkan.h>

namespace graphics::internal
{

struct ImageProperties
{
    VkFormat format; // Data format
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkImageType imageType;
    VkImageViewType imageViewType;
    VkImageLayout finalLayout;
    VkSampleCountFlagBits sampleCount;
    VkSharingMode sharingMode;
    VkImageSubresourceRange imageSubResourceRange;
    VkMemoryPropertyFlags memoryProperties;

    static ImageProperties getDefaultProperties()
    {
        return {
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_SAMPLE_COUNT_1_BIT,
            VK_SHARING_MODE_EXCLUSIVE,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            },
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        };
    }
};

class Image
{
public:
    Image() = default;
    ~Image() = default;

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

private:
    ImageProperties properties;
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
};
} // namespace graphics::internal