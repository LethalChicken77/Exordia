#include "renderer.hpp"
#include "graphics/graphics_data.hpp"
#include "utils/console.hpp"
#include "utils/debug.hpp"

namespace graphics
{
/// @brief Create a renderer with default device and window
Renderer::Renderer() : Renderer(graphicsData->GetBackend().GetDevice(), graphicsData->GetWindow()) {}
/// @brief Create a renderer with specified device and window
/// @param device Device to use for rendering
/// @param window Window to render to
Renderer::Renderer(internal::Device &_device, Window &_window) : device(_device), window(_window){}

Renderer::~Renderer()
{
    vkFreeCommandBuffers(
        device.GetDevice(), 
        device.GetCommandPool(), 
        static_cast<uint32_t>(commandBuffers.size()), 
        commandBuffers.data()
    );
}

void Renderer::init()
{
    RecreateSwapchain();
    createCommandBuffers();
}

VkCommandBuffer Renderer::BeginFrame()
{
    if(frameInProgress)
    {
        throw std::runtime_error("Can't call BeginFrame while already in progress");
    }

    VkResult result = swapchain->AcquireNextImage(&currentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapchain();
        return NULL;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire next image");
    }

    frameInProgress = true;

    currentCommandBuffer = GetCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(currentCommandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffers!");
    }

    return currentCommandBuffer;
}

void Renderer::EndFrame()
{
    if(!frameInProgress)
    {
        throw std::runtime_error("Cannot call EndFrame when frame is not in progress");
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = swapchain->GetImage(currentFrameIndex);
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkCmdPipelineBarrier(
        currentCommandBuffer,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier
    );

    VkResult result = vkEndCommandBuffer(currentCommandBuffer);
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer: " + Debug::VkResultToString(result));
    }

    result = swapchain->SubmitCommandBuffers(&currentCommandBuffer, &currentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.WindowResized())
    {
        window.ResetWindowResizedFlag();
        RecreateSwapchain();
    }
    else if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer");
    }

    frameInProgress = false;
    currentFrameIndex = (currentFrameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;

    currentCommandBuffer = nullptr; // Clear current command buffer pointer, still tracked in vector
}


void Renderer::BeginRenderDynamic(VkCommandBuffer cmdBuffer, VkImageView colorView, VkImageView depthView, VkExtent2D extent, VkClearValue clearColor)
{
    if(!frameInProgress)
    {
        throw std::runtime_error("Cannot begin dynamic rendering when frame is not in progress");
    }
    
    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = colorView;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL; // usually COLOR_ATTACHMENT_OPTIMAL
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = clearColor;

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = depthView;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = {1.0f, 0};

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea.offset = {0, 0};
    renderingInfo.renderArea.extent = extent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;

    vkCmdBeginRendering(cmdBuffer, &renderingInfo);

    // Set viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{{0,0}, extent};

    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
}

void Renderer::EndRenderDynamic(VkCommandBuffer cmdBuffer)
{
    if(!frameInProgress)
    {
        throw std::runtime_error("Cannot end dynamic rendering when frame is not in progress");
    }

    vkCmdEndRendering(cmdBuffer);
}

void Renderer::RecreateSwapchain()
{
    VkExtent2D extent = window.GetExtent();
    while(extent.width == 0 || extent.height == 0)
    {
        extent = window.GetExtent();
        glfwWaitEvents();
    }

    graphicsData->GetBackend().WaitForDevice();

    if(swapchain == nullptr)
    {
        Console::log("Recreating swap chain", "Graphics");
        swapchain = std::make_unique<Swapchain>(
            graphicsData->GetBackend().GetDevice(),
            graphicsData->GetWindow(),
            SwapchainSettings::GetDefaultSettings()
        );
    }
    else
    {
        std::shared_ptr<Swapchain> oldSwapchain = std::move(swapchain);
        Console::log("Resizing swap chain", "Graphics");
        swapchain = std::make_unique<Swapchain>(
            graphicsData->GetBackend().GetDevice(),
            graphicsData->GetWindow(),
            SwapchainSettings::GetDefaultSettings(),
            oldSwapchain->GetSwapchain()
        );

        if(!oldSwapchain->CompareSwapFormats(*swapchain.get()))
        {
            // TODO: Handle this better
            throw std::runtime_error("Swap chain image or depth format has changed!");
        }
    }
}

void Renderer::createCommandBuffers()
{
    #ifdef DEBUG
    Console::debug("Creating command buffers...", "Renderer");
    #endif

    commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
    // currentFrameIndex = 0;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device.GetCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    VkResult result = vkAllocateCommandBuffers(device.GetDevice(), &allocInfo, commandBuffers.data());
    if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers: " + Debug::VkResultToString(result));
    }
}
} // namespace graphics