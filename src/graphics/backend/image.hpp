#pragma once
#include <vulkan/vulkan.h>

#include "core/textureData.hpp"
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

    Image(
        const core::TextureData *textureData,
        const ImageProperties &properties = ImageProperties::getDefaultProperties(),
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        internal::Device &device,
        const core::TextureData *textureData,
        const ImageProperties &properties = ImageProperties::getDefaultProperties(),
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    ~Image();

    Image(const Image&);
    Image(const Image&, internal::Device& device);
    Image& operator=(const Image&) = delete;

    void TransitionImageLayout(VkImageLayout newLayout);
    void TransitionImageLayout(VkImageLayout newLayout, VkCommandBuffer commandBuffer);
    void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer);
    /// @brief Reset to default image layout
    inline void ResetImageLayout() { TransitionImageLayout(defaultLayout); }
    /// @brief Reset to default image layout with command buffer
    inline void ResetImageLayout(VkCommandBuffer commandBuffer) { TransitionImageLayout(defaultLayout, commandBuffer); }

    void SetData(const core::TextureData *data);
    void GetData(const core::TextureData *data);

    /// @brief Get Vulkan descriptor for this image. Does not include a sampler.
    /// @return Vulkan descriptor image info
    VkDescriptorImageInfo GetDescriptorInfo() const
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = currentLayout;
        imageInfo.imageView = imageView;
        imageInfo.sampler = VK_NULL_HANDLE;
        return imageInfo;
    }
private:
    internal::Device &device;

    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo imageAllocationInfo;
    

    ImageProperties properties = ImageProperties::getDefaultProperties();
    VkImageLayout currentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkImageLayout defaultLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkMemoryPropertyFlags memoryPropertyFlags{};

    VkDeviceSize width = 0;
    VkDeviceSize height = 0;
    VkDeviceSize depth = 0;

    void create();
    void createInitialized();
    void createImage();
    void createImageView();
    void copyDataToImage();
};
} // namespace graphics::internal