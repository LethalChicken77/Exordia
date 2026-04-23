#include "swap_chain.hpp"
#include <stdexcept>
#include <limits>
#include <format>

#include "graphics/resources/image.hpp"
#include "utils/console.hpp"
#include "utils/debug.hpp"

namespace graphics
{
    
Swapchain::Swapchain(internal::Device &_device, Window &_window, SwapchainSettings _settings, vk::SwapchainKHR oldSwapchain) : device(_device), window(_window), settings(_settings)
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
        vkDestroyImageView(device.Get(), imageView, nullptr);
    }
    swapchainImageViews.clear();

    if (swapchain != nullptr)
    {
        vkDestroySwapchainKHR(device.Get(), swapchain, nullptr);
        swapchain = nullptr;
    }

    for (int i = 0; i < depthImages.size(); i++)
    {
        vkDestroyImageView(device.Get(), depthImageViews[i], nullptr);
        vmaDestroyImage(device.GetAllocator(), depthImages[i], depthImageAllocations[i]);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(device.Get(), renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device.Get(), imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device.Get(), inFlightFences[i], nullptr);
    }
}

/// @brief Get next image from the swapchain
/// @param imageIndex Pointer to store the image index
/// @return VkResult of the operation
vk::Result Swapchain::AcquireNextImage(uint32_t *imageIndex) 
{
    device.Get().waitForFences(1, &inFlightFences[currentFrame], true, UINT64_MAX);

    return device.Get().acquireNextImageKHR(
        swapchain,
        UINT64_MAX,
        imageAvailableSemaphores[currentFrame],  // must be a not signaled semaphore
        VK_NULL_HANDLE,
        imageIndex
    );

}

/// @brief Submit command buffers to the graphics queue and present the image.
/// @param buffers Command buffers to submit.
/// @param imageIndex
/// @return 
vk::Result Swapchain::SubmitCommandBuffers(const std::span<vk::CommandBuffer> buffers, uint32_t* imageIndex) 
{
    if (imagesInFlight[*imageIndex] != VK_NULL_HANDLE) 
    {
        device.Get().waitForFences(1, &imagesInFlight[*imageIndex], true, UINT64_MAX);
    }
    imagesInFlight[*imageIndex] = inFlightFences[currentFrame];

    vk::SubmitInfo submitInfo{};

    vk::Semaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eTopOfPipe};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &imageAvailableSemaphores[currentFrame];

    submitInfo.commandBufferCount = buffers.size();
    submitInfo.pCommandBuffers = buffers.data();

    vk::Semaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    device.Get().resetFences(1, &inFlightFences[currentFrame]);
    vk::Result result = device.GetGraphicsQueue().submit(1, &submitInfo, inFlightFences[currentFrame]);
    if (result != vk::Result::eSuccess) 
    {
        throw std::runtime_error("Failed to submit draw command buffer: " + Debug::VkResultToString(result));
    }

    vk::PresentInfoKHR presentInfo = {};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = imageIndex;
    presentInfo.pResults = nullptr; // Optional

    vk::SwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    result = device.GetPresentQueue().presentKHR(&presentInfo);

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    return result;
}

void Swapchain::createSwapchain(vk::SwapchainKHR oldSwapchain)
{
    #ifdef DEBUG
    Console::debug("Creating swapchain...", "Swapchain");
    #endif
    internal::PhysicalDevice &physicalDevice = device.GetPhysicalDevice();
    internal::SwapchainSupportDetails swapchainSupport = physicalDevice.QuerySwapChainSupport(&window.GetSurface());

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapchainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapchainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapchainSupport.capabilities);

    uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
    if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) 
    {
        imageCount = swapchainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR createInfo = {};
    createInfo.surface = window.GetSurface();

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

    internal::QueueFamily graphicsFamily = physicalDevice.GetGraphicsFamily();
    internal::QueueFamily presentFamily = physicalDevice.GetPresentFamily();
    if(!presentFamily.IsValid())
    {
        throw std::runtime_error("Failed to create swapchain as present family index is invalid.");
    }
    uint32_t queueFamilyIndices[] = {graphicsFamily.index, presentFamily.index};

    if (graphicsFamily != presentFamily) 
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } 
    else 
    {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapchainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = oldSwapchain;

    VK_CHECK(device.Get().createSwapchainKHR(&createInfo, nullptr, &swapchain), "Failed to create swap chain");

    swapchainImages = device.Get().getSwapchainImagesKHR(swapchain);

    // Transition image formats to avoid branch in render loop
    vk::CommandBuffer cmd = device.BeginSingleTimeCommands();
    for(vk::Image img : swapchainImages)
    {
        Image::TransitionVkImageLayout(
            img,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR,
            cmd,
            {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
        );
    }
    device.EndSingleTimeCommands(cmd);

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
        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = swapchainImages[i];
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = swapchainImageFormat;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(device.Get().createImageView(&viewInfo, nullptr, &swapchainImageViews[i]), "Failed to create swapchain image view");
    }
}

void Swapchain::createDepthImages()
{
    #ifdef DEBUG
    Console::debug("Creating depth images...", "Swapchain");
    #endif
    vk::Format depthFormat = findDepthFormat();
    swapchainDepthFormat = depthFormat;

    depthImages.resize(swapchainImages.size());
    depthImageAllocations.resize(swapchainImages.size());
    depthImageAllocationInfos.resize(swapchainImages.size());
    depthImageViews.resize(swapchainImages.size());

    vk::CommandBuffer commandBuffer = device.BeginSingleTimeCommands();
    for (uint32_t i = 0; i < depthImages.size(); i++) 
    {
        vk::ImageCreateInfo imageInfo{};
        imageInfo.imageType = vk::ImageType::e2D;
        imageInfo.extent.width = swapchainExtent.width;
        imageInfo.extent.height = swapchainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = vk::ImageTiling::eOptimal;
        imageInfo.initialLayout = vk::ImageLayout::eUndefined;
        imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
        imageInfo.samples = vk::SampleCountFlagBits::e1;
        imageInfo.sharingMode = vk::SharingMode::eExclusive;
        imageInfo.flags = vk::ImageCreateFlags();

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        
        vmaCreateImage(device.GetAllocator(), (VkImageCreateInfo*)&imageInfo, &allocInfo, (VkImage*)&depthImages[i], &depthImageAllocations[i], &depthImageAllocationInfos[i]);

        vmaSetAllocationName(device.GetAllocator(), depthImageAllocations[i], "SC Depth");

        vk::ImageViewCreateInfo viewInfo{};
        viewInfo.image = depthImages[i];
        viewInfo.viewType = vk::ImageViewType::e2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VK_CHECK(device.Get().createImageView(&viewInfo, nullptr, &depthImageViews[i]), "Failed to create depth image view");
        Image::TransitionVkImageLayout(
            depthImages[i], 
            VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            commandBuffer,
            viewInfo.subresourceRange);
    }
    device.EndSingleTimeCommands(commandBuffer);
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

    vk::SemaphoreCreateInfo semaphoreInfo = {};

    vk::FenceCreateInfo fenceInfo = {};
    fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
    {
        VK_CHECK(device.Get().createSemaphore(&semaphoreInfo, nullptr, &imageAvailableSemaphores[i]), "Failed to create image available semaphore");
        VK_CHECK(device.Get().createSemaphore(&semaphoreInfo, nullptr, &renderFinishedSemaphores[i]), "Failed to create render finished semaphore");
        VK_CHECK(device.Get().createFence(&fenceInfo, nullptr, &inFlightFences[i]), "Failed to create in-flight fence");
    }
}

vk::SurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats) 
{
    for (const vk::SurfaceFormatKHR &availableFormat : availableFormats) 
    {
        // if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) 
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) 
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Swapchain::chooseSwapPresentMode(const std::set<vk::PresentModeKHR> &availablePresentModes) 
{
    if(settings.overridePresentMode != VK_PRESENT_MODE_MAX_ENUM_KHR &&
        availablePresentModes.contains(vk::PresentModeKHR(settings.overridePresentMode)))
    {
        Console::log("\tPresent mode: Override selected", "Swapchain");
        return vk::PresentModeKHR(settings.overridePresentMode);
    }

    // Really sloppy but it gives the desired prioritzation order
    if(settings.allowMailboxPresentMode)
    {
        if(availablePresentModes.contains(vk::PresentModeKHR::eMailbox))
        {
            Console::log("\tPresent mode: Mailbox", "Swapchain");
            return vk::PresentModeKHR::eMailbox;
        }
    }
    if(!settings.requireVSync)
    {
        if(availablePresentModes.contains(vk::PresentModeKHR::eImmediate))
        {
            Console::log("\tPresent mode: Immediate", "Swapchain");
            return vk::PresentModeKHR::eImmediate;
        }
    }
    if(availablePresentModes.contains(vk::PresentModeKHR::eFifoLatestReady)) 
    {
        Console::log("\tPresent mode: FIFO Latest Ready", "Swapchain");
        return vk::PresentModeKHR::eFifoLatestReady;
    }
    if(!settings.requireVSync)
    {
        if(availablePresentModes.contains(vk::PresentModeKHR::eFifoRelaxed))
        {
            Console::log("\tPresent mode: FIFO Relaxed", "Swapchain");
            return vk::PresentModeKHR::eFifoRelaxed;
        }
    }

    Console::log("\tPresent mode: FIFO", "Swapchain");
    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities) 
{
    if (capabilities.currentExtent.width != UINT32_MAX) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        vk::Extent2D actualExtent = window.GetExtent();
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

vk::Format Swapchain::findDepthFormat() 
{
    return device.GetPhysicalDevice().FindSupportedFormat(
        {vk::Format::eD32Sfloat, vk::Format::eX8D24UnormPack32, vk::Format::eD16Unorm},
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace graphics