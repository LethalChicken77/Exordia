#include "swap_chain.hpp"
#include <stdexcept>
#include <limits>
#include <format>

#include "utils/console.hpp"
#include "utils/debug.hpp"

namespace graphics
{
    
Swapchain::Swapchain(internal::Device &_device, Window &_window, SwapchainSettings _settings, VkSwapchainKHR oldSwapchain) : device(_device), window(_window), settings(_settings)
{
    createSwapchain(oldSwapchain);
    createImageViews();
    createDepthImages();
    createSyncObjects();
    Console::log(std::format("\tNew extent {}x{}", GetSwapChainExtent().width, GetSwapChainExtent().height), "Swapchain");

    // Render pass init order
    // createSwapchain(oldSwapchain);
    // createImageViews();
    // createRenderPass();
    // createDepthResources();
    // createFramebuffers();
    // createSyncObjects();
}

Swapchain::~Swapchain()
{
    for (VkImageView imageView : swapchainImageViews)
    {
        vkDestroyImageView(device.GetDevice(), imageView, nullptr);
    }
    swapchainImageViews.clear();

    if (swapchain != nullptr)
    {
        vkDestroySwapchainKHR(device.GetDevice(), swapchain, nullptr);
        swapchain = nullptr;
    }

    for (int i = 0; i < depthImages.size(); i++)
    {
        vkDestroyImageView(device.GetDevice(), depthImageViews[i], nullptr);
        vmaDestroyImage(device.GetAllocator(), depthImages[i], depthImageAllocations[i]);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device.GetDevice(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.GetDevice(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.GetDevice(), inFlightFences[i], nullptr);
    }
}

/// @brief Get next image from the swapchain
/// @param imageIndex Pointer to store the image index
/// @return VkResult of the operation
VkResult Swapchain::AcquireNextImage(uint32_t *imageIndex) 
{
    vkWaitForFences(
        device.GetDevice(),
        1,
        &inFlightFences[currentFrame],
        VK_TRUE,
        std::numeric_limits<uint64_t>::max()
    );

    VkResult result = vkAcquireNextImageKHR(
        device.GetDevice(),
        swapchain,
        std::numeric_limits<uint64_t>::max(),
        imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
        VK_NULL_HANDLE,
        imageIndex
    );

    return result;
}

/// @brief Submit command buffers to the graphics queue and present the image
/// @param buffers Command buffers to submit
/// @param imageIndex 
/// @return 
VkResult Swapchain::SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex) 
{
    if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) 
    {
        vkWaitForFences(device.GetDevice(), 1, &imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &imageAvailableSemaphores[currentFrame];

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device.GetDevice(), 1, &inFlightFences[currentFrame]);
    VkResult result = vkQueueSubmit(device.GetGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to submit draw command buffer: " + Debug::VkResultToString(result));
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = imageIndex;
    presentInfo.pResults = nullptr; // Optional

    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    result = vkQueuePresentKHR(device.GetPresentQueue(), &presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void Swapchain::createSwapchain(VkSwapchainKHR oldSwapchain)
{
    #ifdef DEBUG
    Console::debug("Creating swapchain...", "Swapchain");
    #endif
    internal::PhysicalDevice &physicalDevice = device.GetPhysicalDevice();
    internal::PhysicalDevice::SwapchainSupportDetails swapchainSupport = physicalDevice.GetSwapchainSupportDetails(&window.GetSurface());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) 
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = window.GetSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    internal::PhysicalDevice::QueueFamilyIndices indices = physicalDevice.GetQueueFamilyIndices();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } 
    else 
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = oldSwapchain;

    VkResult result = vkCreateSwapchainKHR(device.GetDevice(), &createInfo, nullptr, &swapchain);
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("Failed to create swap chain: " + Debug::VkResultToString(result));
    }

    // Query number of images in the swapchain
    vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device.GetDevice(), swapchain, &imageCount, swapchainImages.data());

    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;
}

void Swapchain::createImageViews()
{
    #ifdef DEBUG
    Console::debug("Creating swapchain image views...", "Swapchain");
    #endif
    swapchainImageViews.resize(swapchainImages.size());
    for (size_t i = 0; i < swapchainImages.size(); i++) 
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = swapchainImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = swapchainImageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &swapchainImageViews[i]);
        if(result != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create swapchain image view: " + Debug::VkResultToString(result));
        }
    }
}

void Swapchain::createDepthImages()
{
    #ifdef DEBUG
    Console::debug("Creating depth images...", "Swapchain");
    #endif
    VkFormat depthFormat = findDepthFormat();
    swapchainDepthFormat = depthFormat;

    depthImages.resize(swapchainImages.size());
    depthImageAllocations.resize(swapchainImages.size());
    depthImageAllocationInfos.resize(swapchainImages.size());
    depthImageViews.resize(swapchainImages.size());

    for (uint32_t i = 0; i < depthImages.size(); i++) 
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapchainExtent.width;
        imageInfo.extent.height = swapchainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

        vmaCreateImage(device.GetAllocator(), &imageInfo, &allocInfo, &depthImages[i], &depthImageAllocations[i], &depthImageAllocationInfos[i]);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = depthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkResult result = vkCreateImageView(device.GetDevice(), &viewInfo, nullptr, &depthImageViews[i]);
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create depth image view: " + Debug::VkResultToString(result));
        }
    }
}

void Swapchain::createSyncObjects()
{
    #ifdef DEBUG
    Console::debug("Creating synchronization objects...", "Swapchain");
    #endif
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    imagesInFlight.resize(swapchainImages.size(), VK_NULL_HANDLE);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        VkResult result = vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image available semaphore" + Debug::VkResultToString(result));
        }
        result = vkCreateSemaphore(device.GetDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create render finished semaphore: " + Debug::VkResultToString(result));
        }
        result = vkCreateFence(device.GetDevice(), &fenceInfo, nullptr, &inFlightFences[i]);
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create in-flight fence: " + Debug::VkResultToString(result));
        }
    }
}

VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats) 
{
    for (const VkSurfaceFormatKHR &availableFormat : availableFormats) 
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::set<VkPresentModeKHR> &availablePresentModes) 
{
    if(availablePresentModes.contains(settings.overridePresentMode))
    {
        Console::log("\tPresent mode: Override selected", "Swapchain");
        return settings.overridePresentMode;
    }

    // Really sloppy but it gives the desired prioritzation order
    if(settings.allowMailboxPresentMode)
    {
        if(availablePresentModes.contains(VK_PRESENT_MODE_MAILBOX_KHR))
        {
            Console::log("\tPresent mode: Mailbox", "Swapchain");
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    if(!settings.requireVSync)
    {
        if(availablePresentModes.contains(VK_PRESENT_MODE_IMMEDIATE_KHR))
        {
            Console::log("\tPresent mode: Immediate", "Swapchain");
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }
    if(availablePresentModes.contains(VK_PRESENT_MODE_FIFO_LATEST_READY_EXT)) 
    {
        Console::log("\tPresent mode: FIFO Latest Ready", "Swapchain");
        return VK_PRESENT_MODE_FIFO_LATEST_READY_EXT;
    }
    if(!settings.requireVSync)
    {
        if(availablePresentModes.contains(VK_PRESENT_MODE_FIFO_RELAXED_KHR))
        {
            Console::log("\tPresent mode: FIFO Relaxed", "Swapchain");
            return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        }
    }

    Console::log("\tPresent mode: FIFO", "Swapchain");
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) 
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        VkExtent2D actualExtent = window.GetExtent();
        Console::log(std::format("Choosing swap extent: {}x{}", actualExtent.width, actualExtent.height), "Swapchain");
        actualExtent.width = std::max(
            capabilities.minImageExtent.width,
            std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(
            capabilities.minImageExtent.height,
            std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

VkFormat Swapchain::findDepthFormat() 
{
    return device.GetPhysicalDevice().FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace graphics