#pragma once
#include <vulkan/vulkan.h>

#include "vulkan_backend.hpp"

namespace graphics
{

struct ImageProperties
{
    VkFormat format; // Data format
    VkImageTiling tiling;
    VkImageUsageFlags usage;
    VkImageType imageType;
    VkImageViewType imageViewType;
    VkSampleCountFlagBits sampleCount;
    uint32_t mipLevels;
    uint32_t arrayLayers;
    VkSharingMode sharingMode;
    VkImageSubresourceRange imageSubResourceRange;

    static ImageProperties getDefaultProperties()
    {
        return {
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_IMAGE_TYPE_2D,
            VK_IMAGE_VIEW_TYPE_2D,
            VK_SAMPLE_COUNT_1_BIT,
            1,
            1,
            VK_SHARING_MODE_EXCLUSIVE,
            {
                VK_IMAGE_ASPECT_COLOR_BIT,
                0,
                1,
                0,
                1
            }
        };
    }
};

class Image
{
public:
    Image(
        VkDeviceSize width,
        VkDeviceSize height,
        const ImageProperties &properties = ImageProperties::getDefaultProperties(),
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        internal::Device &device,
        VkDeviceSize width,
        VkDeviceSize height,
        const ImageProperties &properties = ImageProperties::getDefaultProperties(),
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        VkDeviceSize width,
        VkDeviceSize height,
        VkDeviceSize depth,
        const ImageProperties &properties = ImageProperties::getDefaultProperties(),
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        internal::Device &device,
        VkDeviceSize width,
        VkDeviceSize height,
        VkDeviceSize depth,
        const ImageProperties &properties = ImageProperties::getDefaultProperties(),
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    ~Image();

    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    /// @brief Get Vulkan descriptor for this image. Does not include a sampler.
    /// @return Vulkan descriptor image info
    VkDescriptorImageInfo GetDescriptorInfo() const
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = finalLayout;
        imageInfo.imageView = imageView;
        imageInfo.sampler = VK_NULL_HANDLE;
        return imageInfo;
    }
private:
    internal::Device &device;

    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkDeviceMemory imageMemory = VK_NULL_HANDLE;

    ImageProperties properties = ImageProperties::getDefaultProperties();
    VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkMemoryPropertyFlags memoryPropertyFlags{};

    VkDeviceSize width = 0;
    VkDeviceSize height = 0;
    VkDeviceSize depth = 0;

    void createImage();
    void createImageView();
    void copyDataToImage();
};
} // namespace graphics::internal