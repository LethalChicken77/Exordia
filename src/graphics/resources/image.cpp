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

Image::Image(
    const TextureData &textureData,
    const ImageProperties &properties,
    VkMemoryPropertyFlags memoryProperties
) : Image(graphicsData->GetBackend().GetDevice(), textureData.GetWidth(), textureData.GetHeight(), textureData.GetDepth(), properties, memoryProperties)
{
    SetData(textureData);
}

Image::Image(
    internal::Device &_device,
    const TextureData &textureData,
    const ImageProperties &properties,
    VkMemoryPropertyFlags memoryProperties
) : Image(_device, textureData.GetWidth(), textureData.GetHeight(), textureData.GetDepth(), properties, memoryProperties)
{
    SetData(textureData);
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

VkComponentSwizzle getSwizzle(SwizzleChannel swiz)
{
    switch(swiz)
    {
    case SwizzleChannel::Red: return VK_COMPONENT_SWIZZLE_R;
    case SwizzleChannel::Green: return VK_COMPONENT_SWIZZLE_G;
    case SwizzleChannel::Blue: return VK_COMPONENT_SWIZZLE_B;
    case SwizzleChannel::Alpha: return VK_COMPONENT_SWIZZLE_A;
    case SwizzleChannel::Zero: return VK_COMPONENT_SWIZZLE_ONE;
    case SwizzleChannel::One: return VK_COMPONENT_SWIZZLE_ZERO;
    }
}

/// @brief Create the image view for the image
void Image::createImageView()
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = properties.imageViewType;
    viewInfo.format = properties.format;

    // TODO: Handle swizzle from TextureConfig

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

        case VK_FORMAT_R16_SFLOAT: return 2;
        case VK_FORMAT_R16G16_SFLOAT: return 4;
        case VK_FORMAT_R16G16B16_SFLOAT: return 6;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return 8;

        case VK_FORMAT_R32_SFLOAT: return 4;
        case VK_FORMAT_R32G32_SFLOAT: return 8;
        case VK_FORMAT_R32G32B32_SFLOAT: return 12;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;

        // Add more as needed
        default: 
            throw std::runtime_error("Unknown or unsupported VkFormat: " + std::to_string(format));
    }
}

void Image::SetData(const TextureData &data)
{
    format = data.properties.format;
    if(!ImageFormatToVkFormat().contains(format))
    {
        throw std::runtime_error("Unsupported format: " + format.ToString());
    }
    properties.format = ImageFormatToVkFormat().at(data.properties.format);
    
    uint32_t pixelCount = width * height;
    uint32_t pixelSize = format.PixelSize();
    VkDeviceSize bufferSize = pixelSize * pixelCount;
    // std::cout << "Buffer size: " << bufferSize << std::endl;
    // std::cout << "Data size: " << data.size() << std::endl;
    // std::cout << "Width: " << width << " Height: " << height << std::endl;
    if(data.GetSize() != bufferSize)
    {
        Console::error(
            std::format(
                "Texture data size ({}) does not match image size ({})",
                data.GetSize(),
                bufferSize
            ),
            "Image"
        );
        return;
    }
    
    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        device,
        pixelSize,
        pixelCount
    );
    
    stagingBuffer.Map();
    stagingBuffer.WriteData((void *)data.GetDataPtr());

    CopyFromBuffer(stagingBuffer, width, height, 1);
}


void Image::CopyFromBuffer(const Buffer& buffer, uint32_t width, uint32_t height, uint32_t layerCount)
{
    VkImageLayout prevLayout = currentLayout;
    TransitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = layerCount;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer.GetBuffer(),
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);
    device.EndSingleTimeCommands(commandBuffer);
    TransitionImageLayout(prevLayout);
}

void Image::GetData(TextureData *data) const
{
    #ifndef DISABLE_VALIDATION
    if(!data)
    {
        Console::error("No texture data to copy to image", "Image");
        return;
    }
    #endif
    uint32_t pixelCount = width * height;
    uint32_t pixelSize = data->GetComponentSize();
    VkDeviceSize bufferSize = pixelSize * pixelCount;
    if(data->GetSize() != bufferSize)
    {
        throw std::runtime_error("Texture data size does not match buffer size");
    }
    
    Buffer stagingBuffer = Buffer::CreateStagingBuffer(
        device,
        pixelSize,
        pixelCount,
        false
    );
    
    stagingBuffer.Map();

    stagingBuffer.CopyFromImage(*this, width, height, 1);
    
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
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = image.properties.imageSubResourceRange.aspectMask;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 1;
    TransitionVkImageLayout(image.image, oldLayout, newLayout, commandBuffer, subresourceRange);

    image.currentLayout = newLayout;
}

void Image::TransitionVkImageLayout(internal::Device& device, VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange)
{
    VkCommandBuffer commandBuffer = device.BeginSingleTimeCommands(); // Begin recording a command buffer
    TransitionVkImageLayout(image, oldLayout, newLayout, commandBuffer, subresourceRange);
    device.EndSingleTimeCommands(commandBuffer); // Submit and free the command buffer
}

void Image::TransitionVkImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer, VkImageSubresourceRange subresourceRange)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange = subresourceRange;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    switch (oldLayout)
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
}

// struct ImageFormat
// {
    // bool isSRGB = false;
    // uint8_t channelCount = 4;
    // uint8_t channelSize = 1;
    // TextureDataType dataType = TextureDataType::UInt;
    // ChannelOrder channelOrder = ChannelOrder::RGBA;
// };
using DT = TextureDataType;
using CO = ChannelOrder;

const std::unordered_map<ImageFormat, VkFormat, ImageFormat::Hash>& ImageFormatToVkFormat()
{
    static std::unordered_map<ImageFormat, VkFormat, ImageFormat::Hash> if2vf{
        // 8 bit formats
        {{false, 1, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8_UINT},
        {{false, 1, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8_SINT},
        {{false, 1, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8_UNORM},
        {{false, 1, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8_SNORM},
        {{true,  1, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8_SRGB},
        {{true,  1, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8_SRGB},
        {{true,  1, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8_SRGB},
        {{true,  1, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8_SRGB},
        
        {{false, 2, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8G8_UINT},
        {{false, 2, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8G8_SINT},
        {{false, 2, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8G8_UNORM},
        {{false, 2, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8G8_SNORM},
        {{true,  2, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8G8_SRGB},
        {{true,  2, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8G8_SRGB},
        {{true,  2, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8G8_SRGB},
        {{true,  2, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8G8_SRGB},

        {{false, 3, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8G8B8_UINT},
        {{false, 3, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8G8B8_SINT},
        {{false, 3, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8G8B8_UNORM},
        {{false, 3, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8G8B8_SNORM},
        {{true,  3, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8G8B8_SRGB},
        {{true,  3, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8G8B8_SRGB},
        {{true,  3, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8G8B8_SRGB},
        {{true,  3, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8G8B8_SRGB},
        {{false, 3, 1, DT::UInt,  CO::BGRA}, VK_FORMAT_B8G8R8_UINT},
        {{false, 3, 1, DT::SInt,  CO::BGRA}, VK_FORMAT_B8G8R8_SINT},
        {{false, 3, 1, DT::UNorm, CO::BGRA}, VK_FORMAT_B8G8R8_UNORM},
        {{false, 3, 1, DT::SNorm, CO::BGRA}, VK_FORMAT_B8G8R8_SNORM},
        {{true,  3, 1, DT::UInt,  CO::BGRA}, VK_FORMAT_B8G8R8_SRGB},
        {{true,  3, 1, DT::SInt,  CO::BGRA}, VK_FORMAT_B8G8R8_SRGB},
        {{true,  3, 1, DT::UNorm, CO::BGRA}, VK_FORMAT_B8G8R8_SRGB},
        {{true,  3, 1, DT::SNorm, CO::BGRA}, VK_FORMAT_B8G8R8_SRGB},

        {{false, 4, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8G8B8A8_UINT},
        {{false, 4, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8G8B8A8_SINT},
        {{false, 4, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8G8B8A8_UNORM},
        {{false, 4, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8G8B8A8_SNORM},
        {{true,  4, 1, DT::UInt,  CO::RGBA}, VK_FORMAT_R8G8B8A8_SRGB},
        {{true,  4, 1, DT::SInt,  CO::RGBA}, VK_FORMAT_R8G8B8A8_SRGB},
        {{true,  4, 1, DT::UNorm, CO::RGBA}, VK_FORMAT_R8G8B8A8_SRGB},
        {{true,  4, 1, DT::SNorm, CO::RGBA}, VK_FORMAT_R8G8B8A8_SRGB},
        {{false, 4, 1, DT::UInt,  CO::BGRA}, VK_FORMAT_B8G8R8A8_UINT},
        {{false, 4, 1, DT::SInt,  CO::BGRA}, VK_FORMAT_B8G8R8A8_SINT},
        {{false, 4, 1, DT::UNorm, CO::BGRA}, VK_FORMAT_B8G8R8A8_UNORM},
        {{false, 4, 1, DT::SNorm, CO::BGRA}, VK_FORMAT_B8G8R8A8_SNORM},
        {{true,  4, 1, DT::UInt,  CO::BGRA}, VK_FORMAT_B8G8R8A8_SRGB},
        {{true,  4, 1, DT::SInt,  CO::BGRA}, VK_FORMAT_B8G8R8A8_SRGB},
        {{true,  4, 1, DT::UNorm, CO::BGRA}, VK_FORMAT_B8G8R8A8_SRGB},
        {{true,  4, 1, DT::SNorm, CO::BGRA}, VK_FORMAT_B8G8R8A8_SRGB},
        {{false, 4, 1, DT::UInt,  CO::ABGR}, VK_FORMAT_A8B8G8R8_UINT_PACK32},
        {{false, 4, 1, DT::SInt,  CO::ABGR}, VK_FORMAT_A8B8G8R8_SINT_PACK32},
        {{false, 4, 1, DT::UNorm, CO::ABGR}, VK_FORMAT_A8B8G8R8_UNORM_PACK32},
        {{false, 4, 1, DT::SNorm, CO::ABGR}, VK_FORMAT_A8B8G8R8_SNORM_PACK32},
        {{true,  4, 1, DT::UInt,  CO::ABGR}, VK_FORMAT_A8B8G8R8_SRGB_PACK32},
        {{true,  4, 1, DT::SInt,  CO::ABGR}, VK_FORMAT_A8B8G8R8_SRGB_PACK32},
        {{true,  4, 1, DT::UNorm, CO::ABGR}, VK_FORMAT_A8B8G8R8_SRGB_PACK32},
        {{true,  4, 1, DT::SNorm, CO::ABGR}, VK_FORMAT_A8B8G8R8_SRGB_PACK32},
        // 16 bit formats
        {{false, 1, 2, DT::UInt,  CO::RGBA}, VK_FORMAT_R16_UINT},
        {{false, 1, 2, DT::SInt,  CO::RGBA}, VK_FORMAT_R16_SINT},
        {{false, 1, 2, DT::UNorm, CO::RGBA}, VK_FORMAT_R16_UNORM},
        {{false, 1, 2, DT::SNorm, CO::RGBA}, VK_FORMAT_R16_SNORM},
        {{false, 1, 2, DT::Float, CO::RGBA}, VK_FORMAT_R16_SFLOAT},

        {{false, 2, 2, DT::UInt,  CO::RGBA}, VK_FORMAT_R16G16_UINT},
        {{false, 2, 2, DT::SInt,  CO::RGBA}, VK_FORMAT_R16G16_SINT},
        {{false, 2, 2, DT::UNorm, CO::RGBA}, VK_FORMAT_R16G16_UNORM},
        {{false, 2, 2, DT::SNorm, CO::RGBA}, VK_FORMAT_R16G16_SNORM},
        {{false, 2, 2, DT::Float, CO::RGBA}, VK_FORMAT_R16G16_SFLOAT},
        
        {{false, 3, 2, DT::UInt,  CO::RGBA}, VK_FORMAT_R16G16B16_UINT},
        {{false, 3, 2, DT::SInt,  CO::RGBA}, VK_FORMAT_R16G16B16_SINT},
        {{false, 3, 2, DT::UNorm, CO::RGBA}, VK_FORMAT_R16G16B16_UNORM},
        {{false, 3, 2, DT::SNorm, CO::RGBA}, VK_FORMAT_R16G16B16_SNORM},
        {{false, 3, 2, DT::Float, CO::RGBA}, VK_FORMAT_R16G16B16_SFLOAT},
        
        {{false, 4, 2, DT::UInt,  CO::RGBA}, VK_FORMAT_R16G16B16A16_UINT},
        {{false, 4, 2, DT::SInt,  CO::RGBA}, VK_FORMAT_R16G16B16A16_SINT},
        {{false, 4, 2, DT::UNorm, CO::RGBA}, VK_FORMAT_R16G16B16A16_UNORM},
        {{false, 4, 2, DT::SNorm, CO::RGBA}, VK_FORMAT_R16G16B16A16_SNORM},
        {{false, 4, 2, DT::Float, CO::RGBA}, VK_FORMAT_R16G16B16A16_SFLOAT},
        // 32 bit formats
        {{false, 1, 4, DT::UInt,  CO::RGBA}, VK_FORMAT_R32_UINT},
        {{false, 1, 4, DT::SInt,  CO::RGBA}, VK_FORMAT_R32_SINT},
        {{false, 1, 4, DT::Float, CO::RGBA}, VK_FORMAT_R32_SFLOAT},

        {{false, 2, 4, DT::UInt,  CO::RGBA}, VK_FORMAT_R32G32_UINT},
        {{false, 2, 4, DT::SInt,  CO::RGBA}, VK_FORMAT_R32G32_SINT},
        {{false, 2, 4, DT::Float, CO::RGBA}, VK_FORMAT_R32G32_SFLOAT},
        
        {{false, 3, 4, DT::UInt,  CO::RGBA}, VK_FORMAT_R32G32B32_UINT},
        {{false, 3, 4, DT::SInt,  CO::RGBA}, VK_FORMAT_R32G32B32_SINT},
        {{false, 3, 4, DT::Float, CO::RGBA}, VK_FORMAT_R32G32B32_SFLOAT},
        
        {{false, 4, 4, DT::UInt,  CO::RGBA}, VK_FORMAT_R32G32B32A32_UINT},
        {{false, 4, 4, DT::SInt,  CO::RGBA}, VK_FORMAT_R32G32B32A32_SINT},
        {{false, 4, 4, DT::Float, CO::RGBA}, VK_FORMAT_R32G32B32A32_SFLOAT},
        // 64 bit formats
        {{false, 1, 8, DT::UInt,  CO::RGBA}, VK_FORMAT_R64_UINT},
        {{false, 1, 8, DT::SInt,  CO::RGBA}, VK_FORMAT_R64_SINT},
        {{false, 1, 8, DT::Float, CO::RGBA}, VK_FORMAT_R64_SFLOAT},

        {{false, 2, 8, DT::UInt,  CO::RGBA}, VK_FORMAT_R64G64_UINT},
        {{false, 2, 8, DT::SInt,  CO::RGBA}, VK_FORMAT_R64G64_SINT},
        {{false, 2, 8, DT::Float, CO::RGBA}, VK_FORMAT_R64G64_SFLOAT},
        
        {{false, 3, 8, DT::UInt,  CO::RGBA}, VK_FORMAT_R64G64B64_UINT},
        {{false, 3, 8, DT::SInt,  CO::RGBA}, VK_FORMAT_R64G64B64_SINT},
        {{false, 3, 8, DT::Float, CO::RGBA}, VK_FORMAT_R64G64B64_SFLOAT},
        
        {{false, 4, 8, DT::UInt,  CO::RGBA}, VK_FORMAT_R64G64B64A64_UINT},
        {{false, 4, 8, DT::SInt,  CO::RGBA}, VK_FORMAT_R64G64B64A64_SINT},
        {{false, 4, 8, DT::Float, CO::RGBA}, VK_FORMAT_R64G64B64A64_SFLOAT},

        // Packed formats
        {{false, 1, 4, DT::Packed_B10G11R11_UFloat,  CO::RGBA}, VK_FORMAT_B10G11R11_UFLOAT_PACK32},
        {{false, 1, 4, DT::Packed_B10G11R11_UFloat,  CO::BGRA}, VK_FORMAT_B10G11R11_UFLOAT_PACK32}, // In case user explicitly sets channel order
        {{false, 1, 4, DT::Packed_A2R10G10B10_UNorm, CO::RGBA}, VK_FORMAT_A2R10G10B10_UNORM_PACK32},
        
        // Depth formats
        {{false, 1, 2, DT::D16_UNorm,          CO::RGBA}, VK_FORMAT_D16_UNORM},
        {{false, 1, 4, DT::D24_UNorm,          CO::RGBA}, VK_FORMAT_X8_D24_UNORM_PACK32},
        {{false, 1, 4, DT::D32_SFloat,         CO::RGBA}, VK_FORMAT_D32_SFLOAT},
        {{false, 1, 4, DT::D16_UNorm_S8_UInt,  CO::RGBA}, VK_FORMAT_D16_UNORM_S8_UINT},
        {{false, 1, 4, DT::D24_UNorm,          CO::RGBA}, VK_FORMAT_D24_UNORM_S8_UINT},
        {{false, 1, 4, DT::D32_SFloat_S8_UInt, CO::RGBA}, VK_FORMAT_D32_SFLOAT_S8_UINT},
        {{false, 1, 4, DT::S8_UInt,            CO::RGBA}, VK_FORMAT_S8_UINT},

    };
    return if2vf;
}

const std::unordered_map<VkFormat, ImageFormat>& VkFormatToImageFormat()
{
    static std::unordered_map<VkFormat, ImageFormat> vf2if{
        // 8 bit formats
        {VK_FORMAT_R8_UINT,  {false, 1, 1, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R8_SINT,  {false, 1, 1, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R8_UNORM, {false, 1, 1, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R8_SNORM, {false, 1, 1, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R8_SRGB,  {true,  1, 1, DT::UNorm, CO::RGBA}},
        
        {VK_FORMAT_R8G8_UINT,  {false, 2, 1, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R8G8_SINT,  {false, 2, 1, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R8G8_UNORM, {false, 2, 1, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R8G8_SNORM, {false, 2, 1, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R8G8_SRGB,  {true,  2, 1, DT::UNorm, CO::RGBA}},

        {VK_FORMAT_R8G8B8_UINT,  {false, 3, 1, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R8G8B8_SINT,  {false, 3, 1, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R8G8B8_UNORM, {false, 3, 1, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R8G8B8_SNORM, {false, 3, 1, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R8G8B8_SRGB,  {true,  3, 1, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_B8G8R8_UINT,  {false, 3, 1, DT::UInt,  CO::BGRA}},
        {VK_FORMAT_B8G8R8_SINT,  {false, 3, 1, DT::SInt,  CO::BGRA}},
        {VK_FORMAT_B8G8R8_UNORM, {false, 3, 1, DT::UNorm, CO::BGRA}},
        {VK_FORMAT_B8G8R8_SNORM, {false, 3, 1, DT::SNorm, CO::BGRA}},
        {VK_FORMAT_B8G8R8_SRGB,  {true,  3, 1, DT::UNorm, CO::BGRA}},

        {VK_FORMAT_R8G8B8A8_UINT,  {false, 4, 1, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R8G8B8A8_SINT,  {false, 4, 1, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R8G8B8A8_UNORM, {false, 4, 1, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R8G8B8A8_SNORM, {false, 4, 1, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R8G8B8A8_SRGB,  {true,  4, 1, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_B8G8R8A8_UINT,  {false, 4, 1, DT::UInt,  CO::BGRA}},
        {VK_FORMAT_B8G8R8A8_SINT,  {false, 4, 1, DT::SInt,  CO::BGRA}},
        {VK_FORMAT_B8G8R8A8_UNORM, {false, 4, 1, DT::UNorm, CO::BGRA}},
        {VK_FORMAT_B8G8R8A8_SNORM, {false, 4, 1, DT::SNorm, CO::BGRA}},
        {VK_FORMAT_B8G8R8A8_SRGB,  {true,  4, 1, DT::UNorm, CO::BGRA}},
        {VK_FORMAT_A8B8G8R8_UINT_PACK32,  {false, 4, 1, DT::UInt,  CO::ABGR}},
        {VK_FORMAT_A8B8G8R8_SINT_PACK32,  {false, 4, 1, DT::SInt,  CO::ABGR}},
        {VK_FORMAT_A8B8G8R8_UNORM_PACK32, {false, 4, 1, DT::UNorm, CO::ABGR}},
        {VK_FORMAT_A8B8G8R8_SNORM_PACK32, {false, 4, 1, DT::SNorm, CO::ABGR}},
        {VK_FORMAT_A8B8G8R8_SRGB_PACK32,  {true,  4, 1, DT::UNorm, CO::ABGR}},
        // 16 bit formats
        {VK_FORMAT_R16_UINT,   {false, 1, 2, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R16_SINT,   {false, 1, 2, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R16_UNORM,  {false, 1, 2, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R16_SNORM,  {false, 1, 2, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R16_SFLOAT, {false, 1, 2, DT::Float, CO::RGBA}},

        {VK_FORMAT_R16G16_UINT,   {false, 2, 2, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R16G16_SINT,   {false, 2, 2, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R16G16_UNORM,  {false, 2, 2, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R16G16_SNORM,  {false, 2, 2, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R16G16_SFLOAT, {false, 2, 2, DT::Float, CO::RGBA}},
        
        {VK_FORMAT_R16G16B16_UINT,   {false, 3, 2, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R16G16B16_SINT,   {false, 3, 2, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R16G16B16_UNORM,  {false, 3, 2, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R16G16B16_SNORM,  {false, 3, 2, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R16G16B16_SFLOAT, {false, 3, 2, DT::Float, CO::RGBA}},
        
        {VK_FORMAT_R16G16B16A16_UINT,   {false, 4, 2, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R16G16B16A16_SINT,   {false, 4, 2, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R16G16B16A16_UNORM,  {false, 4, 2, DT::UNorm, CO::RGBA}},
        {VK_FORMAT_R16G16B16A16_SNORM , {false, 4, 2, DT::SNorm, CO::RGBA}},
        {VK_FORMAT_R16G16B16A16_SFLOAT, {false, 4, 2, DT::Float, CO::RGBA}},
        // 32 bit formats
        {VK_FORMAT_R32_UINT,   {false, 1, 4, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R32_SINT,   {false, 1, 4, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R32_SFLOAT, {false, 1, 4, DT::Float, CO::RGBA}},

        {VK_FORMAT_R32G32_UINT,   {false, 2, 4, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R32G32_SINT,   {false, 2, 4, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R32G32_SFLOAT, {false, 2, 4, DT::Float, CO::RGBA}},
        
        {VK_FORMAT_R32G32B32_UINT,   {false, 3, 4, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R32G32B32_SINT,   {false, 3, 4, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R32G32B32_SFLOAT, {false, 3, 4, DT::Float, CO::RGBA}},
        
        {VK_FORMAT_R32G32B32A32_UINT,   {false, 4, 4, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R32G32B32A32_SINT,   {false, 4, 4, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R32G32B32A32_SFLOAT, {false, 4, 4, DT::Float, CO::RGBA}},
        // 64 bit formats
        {VK_FORMAT_R64_UINT,   {false, 1, 8, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R64_SINT,   {false, 1, 8, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R64_SFLOAT, {false, 1, 8, DT::Float, CO::RGBA}},

        {VK_FORMAT_R64G64_UINT,   {false, 2, 8, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R64G64_SINT,   {false, 2, 8, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R64G64_SFLOAT, {false, 2, 8, DT::Float, CO::RGBA}},
        
        {VK_FORMAT_R64G64B64_UINT,   {false, 3, 8, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R64G64B64_SINT,   {false, 3, 8, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R64G64B64_SFLOAT, {false, 3, 8, DT::Float, CO::RGBA}},
        
        {VK_FORMAT_R64G64B64A64_UINT,   {false, 4, 8, DT::UInt,  CO::RGBA}},
        {VK_FORMAT_R64G64B64A64_SINT,   {false, 4, 8, DT::SInt,  CO::RGBA}},
        {VK_FORMAT_R64G64B64A64_SFLOAT, {false, 4, 8, DT::Float, CO::RGBA}},

        // Packed formats
        {VK_FORMAT_B10G11R11_UFLOAT_PACK32,  {false, 1, 4, DT::Packed_B10G11R11_UFloat,  CO::RGBA}},
        {VK_FORMAT_B10G11R11_UFLOAT_PACK32,  {false, 1, 4, DT::Packed_B10G11R11_UFloat,  CO::BGRA}}, // In case user explicitly sets channel order
        {VK_FORMAT_A2R10G10B10_UNORM_PACK32, {false, 1, 4, DT::Packed_A2R10G10B10_UNorm, CO::RGBA}},
        
        // Depth formats
        {VK_FORMAT_D16_UNORM,           {false, 1, 2, DT::D16_UNorm,          CO::RGBA}},
        {VK_FORMAT_X8_D24_UNORM_PACK32, {false, 1, 4, DT::D24_UNorm,          CO::RGBA}},
        {VK_FORMAT_D32_SFLOAT,          {false, 1, 4, DT::D32_SFloat,         CO::RGBA}},
        {VK_FORMAT_D16_UNORM_S8_UINT,   {false, 1, 4, DT::D16_UNorm_S8_UInt,  CO::RGBA}},
        {VK_FORMAT_D24_UNORM_S8_UINT,   {false, 1, 4, DT::D24_UNorm,          CO::RGBA}},
        {VK_FORMAT_D32_SFLOAT_S8_UINT,  {false, 1, 4, DT::D32_SFloat_S8_UInt, CO::RGBA}},
        {VK_FORMAT_S8_UINT,             {false, 1, 4, DT::S8_UInt,            CO::RGBA}},

    };
    return vf2if;
}

}