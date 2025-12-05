#include "image.hpp"
#include "graphics/graphics_data.hpp"

#include <format>
#include "utils/console.hpp"

namespace graphics
{
Image::Image(
    VkDeviceSize width,
    VkDeviceSize height,
    const ImageProperties &properties,
    VkMemoryPropertyFlags memoryProperties
    ) : Image(
        graphicsData->GetBackend().GetDevice(),
        width,
        height,
        1,
        properties,
        memoryProperties) {}

Image::Image(
    internal::Device &device,
    VkDeviceSize width,
    VkDeviceSize height,
    const ImageProperties &properties,
    VkMemoryPropertyFlags memoryProperties
    ) : Image(
        device,
        width,
        height,
        1,
        properties,
        memoryProperties) {}

Image::Image(
    VkDeviceSize width,
    VkDeviceSize height,
    VkDeviceSize depth,
    const ImageProperties &properties,
    VkMemoryPropertyFlags memoryProperties
    ) : Image(
        graphicsData->GetBackend().GetDevice(),
        width,
        height,
        depth,
        properties,
        memoryProperties) {}

Image::Image(
    internal::Device &_device,
    VkDeviceSize _width,
    VkDeviceSize _height,
    VkDeviceSize _depth,
    const ImageProperties &_properties,
    VkMemoryPropertyFlags _memoryProperties
) : device(_device)
{
    width = _width;
    height = _height;
    depth = _depth;
    properties = _properties;
    memoryPropertyFlags = _memoryProperties;

    #ifdef DEBUG
    Console::log(std::format("Creating image: width: {}, height: {}, depth: {}", _width, _height, _depth), "Image");
    #endif

    createImage();
    // createImageView();
    // copyDataToImage();
    // createDescriptorInfo();
}

Image::~Image()
{
    #ifndef DISABLE_VALIDATION
    if(imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device.GetDevice(), imageView, nullptr);
    }
    if(image != VK_NULL_HANDLE)
    {
        vkDestroyImage(device.GetDevice(), image, nullptr);
    }
    if(imageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device.GetDevice(), imageMemory, nullptr);
    }
    #else
    vkDestroyImageView(device.GetDevice(), imageView, nullptr);
    vkDestroyImage(device.GetDevice(), image, nullptr);
    vkFreeMemory(device.GetDevice(), imageMemory, nullptr);
    #endif
}

void Image::createImage()
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = properties.imageType;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = depth;
    imageInfo.mipLevels = properties.mipLevels;
    imageInfo.arrayLayers = properties.arrayLayers;
    imageInfo.format = properties.format;
    imageInfo.tiling = properties.tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = properties.usage;
    imageInfo.samples = properties.sampleCount;
    imageInfo.sharingMode = properties.sharingMode;
    
    device.CreateImage(
        width,
        height,
        depth,
        imageInfo,
        memoryPropertyFlags,
        image,
        imageMemory
    );
}
}