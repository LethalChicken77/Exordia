#pragma once
#include <unordered_map>

#include "graphics/backend/vulkan_include.h"

#include "graphics/api/resources/texture_data.hpp"
#include "graphics/backend/device.hpp"
#include "buffer.hpp"

namespace graphics
{

extern const std::unordered_map<ImageFormat, VkFormat, ImageFormatHash> imageFormatToVkFormat;

struct ImageProperties
{
    VkFormat format = VK_FORMAT_R8G8B8A8_SRGB; // Data format
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageType imageType = VK_IMAGE_TYPE_2D;
    VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;
    VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImageSubresourceRange imageSubResourceRange{
        VK_IMAGE_ASPECT_COLOR_BIT,
        0,
        1,
        0,
        1
    };

    [[deprecated]]
    static ImageProperties getDefaultProperties()
    {
        return ImageProperties();
    }
};

class Image
{
public:
    Image(
        VkDeviceSize width,
        VkDeviceSize height,
        const ImageProperties &properties = {},
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        internal::Device &device,
        VkDeviceSize width,
        VkDeviceSize height,
        const ImageProperties &properties = {},
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        VkDeviceSize width,
        VkDeviceSize height,
        VkDeviceSize depth,
        const ImageProperties &properties = {},
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        internal::Device &device,
        VkDeviceSize width,
        VkDeviceSize height,
        VkDeviceSize depth,
        const ImageProperties &properties = {},
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    Image(
        const TextureData &textureData,
        const ImageProperties &properties = {},
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );
    Image(
        internal::Device &device,
        const TextureData &textureData,
        const ImageProperties &properties = {},
        VkMemoryPropertyFlags memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    ~Image();

    Image(const Image&);
    Image(const Image&, internal::Device& device);
    Image& operator=(const Image&) = delete;

    static void TransitionImageLayout(Image& image, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer);

    void TransitionImageLayout(VkImageLayout newLayout);
    void TransitionImageLayout(VkImageLayout newLayout, VkCommandBuffer commandBuffer);
    void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer);
    /// @brief Reset to default image layout
    inline void ResetImageLayout() { TransitionImageLayout(defaultLayout); }
    /// @brief Reset to default image layout with command buffer
    inline void ResetImageLayout(VkCommandBuffer commandBuffer) { TransitionImageLayout(defaultLayout, commandBuffer); }

    void CopyFromBuffer(const Buffer& buffer, uint32_t width, uint32_t height, uint32_t layerCount);

    void SetData(const TextureData &data);
    void GetData(TextureData *data) const;
    VkImage GetImage() const { return image; }
    VkImageView GetImageView() const { return imageView; }


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
    ImageFormat format{};

    VkImage image = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VmaAllocationInfo imageAllocationInfo;
    
    ImageProperties properties{};
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

    friend class Buffer;
};
} // namespace graphics::internal