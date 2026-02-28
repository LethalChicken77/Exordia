#include "image.hpp"
#include "graphics/graphics_data.hpp"

#include <format>
#include "utils/console.hpp"
#include "buffer.hpp"

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
    Console::debug(std::format("Creating image: width: {}, height: {}, depth: {}", _width, _height, _depth), "Image");
    #endif

    create();
}

/// @brief Create a copy of an existing image on the same device
/// @param other 
/// @note New image shares the same image data. Can be changed with SetCPUData later as long as the resolution matches. Use with caution.
Image::Image(const Image& other) : device(other.device)
{
    width = other.width;
    height = other.height;
    depth = other.depth;
    properties = other.properties;
    memoryPropertyFlags = other.memoryPropertyFlags;

    create();
}

/// @brief Create a copy of an existing image on a specified device
/// @param other 
/// @param device New device to create the image on
/// @note New image shares the same image data. Can be changed with SetCPUData later as long as the resolution matches. Use with caution.
Image::Image(const Image& other, internal::Device& _device) : device(_device)
{
    width = other.width;
    height = other.height;
    depth = other.depth;
    properties = other.properties;
    memoryPropertyFlags = other.memoryPropertyFlags;

    create();
}

// Image::Image& operator=(const Image&)

Image::~Image()
{
    #ifndef DISABLE_VALIDATION
    if(imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device.Get(), imageView, nullptr);
    }
    if(imageAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyImage(device.GetAllocator(), image, imageAllocation);
    }
    #else
    vkDestroyImageView(device.GetDevice(), imageView, nullptr);
    vmaDestroyImage(device.GetAllocator(), image, imageAllocation);
    #endif
}

/// @brief Create the Vulkan image
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
    
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    // allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT; // For large images
    // allocCreateInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT; // Persistent mapping
    vmaCreateImage(
        device.GetAllocator(),
        &imageInfo,
        &allocCreateInfo,
        &image,
        &imageAllocation,
        &imageAllocationInfo
    );
}

/// @brief Create the image view for the image
void Image::createImageView()
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = properties.imageViewType;
    viewInfo.format = properties.format;
    // viewInfo.subresourceRange = properties.imageSubResourceRange;
    viewInfo.subresourceRange.aspectMask = properties.imageSubResourceRange.aspectMask;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if(vkCreateImageView(device.Get(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture image view");
    }
}

/// @brief Create image without initializing with data
void Image::create()
{
    createImage();
    createImageView();
    TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, defaultLayout);
}

/// @brief Create image and initialize with data from textureData
void Image::createInitialized()
{
    createImage();
    createImageView();
    TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    // copyDataToImage();
    TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, defaultLayout);
}

size_t getFormatSize(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R8_UNORM: return 1;

        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R8G8_UNORM: return 2;

        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_R8G8B8_UNORM: return 3;

        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_UNORM: return 4;

        case VK_FORMAT_R32_SFLOAT: return 4;
        case VK_FORMAT_R32G32_SFLOAT: return 8;
        case VK_FORMAT_R32G32B32_SFLOAT: return 12;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;

        // Add more as needed
        default: 
            throw std::runtime_error("Unknown or unsupported VkFormat: " + std::to_string(format));
    }
}

void Image::SetData(const core::TextureData *data)
{
    #ifndef DISABLE_VALIDATION
    if(!data)
    {
        Console::error("No texture data to copy to image", "Image");
        return;
    }
    #endif
    uint32_t pixelCount = width * height;
    uint32_t pixelSize = getFormatSize(properties.format);
    VkDeviceSize bufferSize = pixelSize * pixelCount;
    // std::cout << "Buffer size: " << bufferSize << std::endl;
    // std::cout << "Data size: " << data.size() << std::endl;
    // std::cout << "Width: " << width << " Height: " << height << std::endl;
    if(data->GetSize() != bufferSize)
    {
        Console::error(
            std::format(
                "Texture data size ({}) does not match buffer size ({})",
                data->GetSize(),
                bufferSize
            ),
            "Image"
        );
        return;
    }
    
    Buffer stagingBuffer{
        device,
        pixelSize,
        pixelCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };
    
    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)data->GetDataPtr());
    
    // device->copyBufferToImage(stagingBuffer.getBuffer(), image, width, height, 1);
}

void Image::GetData(const core::TextureData *data)
{
    #ifndef DISABLE_VALIDATION
    if(!data)
    {
        Console::error("No texture data to copy to image", "Image");
        return;
    }
    #endif
    uint32_t pixelCount = width * height;
    uint32_t pixelSize = sizeof(data[0]) * 4;
    VkDeviceSize bufferSize = pixelSize * pixelCount;
    if(data->GetSize() != bufferSize)
    {
        throw std::runtime_error("Texture data size does not match buffer size");
    }
    
    Buffer stagingBuffer{
        device,
        pixelSize,
        pixelCount,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };
    
    stagingBuffer.Map();
    
    // device->copyImageToBuffer(image, stagingBuffer.getBuffer(), width, height, 1);
    
    stagingBuffer.ReadData((void *)data->GetDataPtr());
}

/// @brief Change the layout of the image
/// @param newLayout New layout to transition to
void Image::TransitionImageLayout(VkImageLayout newLayout)
{
    TransitionImageLayout(currentLayout, newLayout);
}

/// @brief Change the layout of the image using a provided command buffer
/// @param newLayout New layout to transition to
/// @param commmandBuffer Command buffer to use for the transition
void Image::TransitionImageLayout(VkImageLayout newLayout, VkCommandBuffer commmandBuffer)
{
    TransitionImageLayout(currentLayout, newLayout, commmandBuffer);
}

/// @brief Transition the image layout
/// @param oldLayout
/// @param newLayout
void Image::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands(); // Begin recording a command buffer
    TransitionImageLayout(oldLayout, newLayout, commandBuffer);
    device.EndSingleTimeCommands(commandBuffer); // Submit and free the command buffer
}

/// @brief Transition the image layout using a provided command buffer
/// @param oldLayout
/// @param newLayout 
/// @param commandBuffer Command buffer to use for the transition
void Image::TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
{
    TransitionImageLayout(*this, oldLayout, newLayout, commandBuffer);
}

/// @brief Transition the image layout using a provided command buffer
/// @param image Image to transition
/// @param oldLayout
/// @param newLayout 
/// @param commandBuffer Command buffer to use for the transition
void Image::TransitionImageLayout(Image& image, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image.image;
    barrier.subresourceRange.aspectMask = image.properties.imageSubResourceRange.aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    switch(oldLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
            barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        default:
            Console::error("Unsupported old layout transition: " + std::to_string((uint32_t)oldLayout), "Image");
            barrier.srcAccessMask = 0;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
    }

    switch(newLayout)
    {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.dstAccessMask = 0;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_GENERAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_HOST_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            break;
        case VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            break;
        case VK_IMAGE_LAYOUT_RENDERING_LOCAL_READ:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_DECODE_DPB_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
        case VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR:
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_DPB_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        case VK_IMAGE_LAYOUT_ATTACHMENT_FEEDBACK_LOOP_OPTIMAL_EXT:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
            break;
        case VK_IMAGE_LAYOUT_VIDEO_ENCODE_QUANTIZATION_MAP_KHR:
            barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            destinationStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            break;
        default:
            Console::error("Unsupported new layout transition: " + std::to_string((uint32_t)oldLayout), "Image");
            barrier.dstAccessMask = 0;
            destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            break;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    image.currentLayout = newLayout;
}

}