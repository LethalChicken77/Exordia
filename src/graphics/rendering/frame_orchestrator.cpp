#include "frame_orchestrator.hpp"
#include "graphics/graphics_data.hpp"
#include "utils/console.hpp"
#include "utils/debug.hpp"
#include "graphics/resources/image.hpp"

namespace graphics
{
/// @brief Create a renderer with default device and window
FrameOrchestrator::FrameOrchestrator() : FrameOrchestrator(graphicsData->GetBackend().GetDevice(), graphicsData->GetWindow()) {}
/// @brief Create a renderer with specified device and window
/// @param device Device to use for rendering
/// @param window Window to render to
FrameOrchestrator::FrameOrchestrator(internal::Device &_device, Window *_window) : device(_device), window(_window)
{
    RecreateSwapchain();
    createCommandBuffers();
}

FrameOrchestrator::~FrameOrchestrator()
{
}

void FrameOrchestrator::Cleanup()
{
    swapchain.reset();
    device.Get().freeCommandBuffers(
        device.GetCommandPool(),
        commandBuffers.size(),
        commandBuffers.data()
    );
}

FrameContext FrameOrchestrator::BeginFrame()
{
    if(frameInProgress)
    {
        throw std::runtime_error("Can't call BeginFrame while already in progress");
    }

    vk::Result result = swapchain->AcquireNextImage(&currentImageIndex);
    if(result == vk::Result::eErrorOutOfDateKHR)
    {
        RecreateSwapchain();
        return FrameContext();
    }

    if(result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR) // TODO: Handle suboptimal KHR
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

    return GetContext();
}

void FrameOrchestrator::EndFrame()
{
    #ifdef DEBUG // Skip extra branches on release builds
    if(!frameInProgress)
    {
        throw std::runtime_error("Cannot call EndFrame when frame is not in progress");
    }
    #endif

    transitionToPresent(currentCommandBuffer, swapchain->GetImage(currentImageIndex));

    #ifdef VULKAN_HPP_NO_EXCEPTIONS
    VK_CHECK(currentCommandBuffer.end(), "Failed to record command buffer");
    #else
    currentCommandBuffer.end();
    #endif

    vk::Result result = swapchain->SubmitCommandBuffers({&currentCommandBuffer, 1}, &currentImageIndex);
    if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || window->WindowResized())
    {
        window->ResetWindowResizedFlag();
        RecreateSwapchain();
        if(result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) // Allows me to use VK_CHECK to print unhandled errors
            result = vk::Result::eSuccess;
    }
    VK_CHECK(result, "Failed to submit command buffer");

    frameInProgress = false;
    currentFrameIndex = (currentFrameIndex + 1) % Swapchain::MAX_FRAMES_IN_FLIGHT;

    currentCommandBuffer = nullptr; // Clear current command buffer pointer, still tracked in vector
}


void FrameOrchestrator::BeginRenderDynamic(const FrameContext& context, const PassInfo& passInfo)
{
    if(!frameInProgress)
    {
        throw std::runtime_error("Cannot begin dynamic rendering when frame is not in progress");
    }
    
    vk::RenderingAttachmentInfo colorAttachment{};
    colorAttachment.imageView = passInfo.colorView;
    colorAttachment.imageLayout = vk::ImageLayout::eAttachmentOptimal; // usually COLOR_ATTACHMENT_OPTIMAL
    colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    colorAttachment.clearValue = passInfo.clearColor;

    vk::RenderingAttachmentInfo depthAttachment{};
    depthAttachment.imageView = passInfo.depthView;
    depthAttachment.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.clearValue.depthStencil = vk::ClearDepthStencilValue{graphicsData->REVERSED_DEPTH ? 0.0f : 1.0f, 0};

    vk::RenderingInfo renderingInfo{};
    renderingInfo.renderArea.offset = vk::Offset2D{0, 0};
    renderingInfo.renderArea.extent = passInfo.extent;
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;

    transitionToRenderTarget(context.commandBuffer, swapchain->GetImage(currentImageIndex));
    
    context.commandBuffer.beginRendering(&renderingInfo);

    // Set viewport and scissor
    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(passInfo.extent.width);
    viewport.height = static_cast<float>(passInfo.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor{{0,0}, passInfo.extent};

    context.commandBuffer.setViewport(0, 1, &viewport);
    context.commandBuffer.setScissor(0, 1, &scissor);
}

void FrameOrchestrator::EndRenderDynamic(const FrameContext& context, const PassInfo& passInfo)
{
    if(!frameInProgress)
    {
        throw std::runtime_error("Cannot end dynamic rendering when frame is not in progress");
    }

    context.commandBuffer.endRendering();
}

void FrameOrchestrator::RecreateSwapchain()
{
    vk::Extent2D extent = window->GetExtent();
    Console::log(std::format("Recreating swap chain with extent {}x{}", extent.width, extent.height), "FrameOrchestrator");
    while(extent.width == 0 || extent.height == 0)
    {
        extent = window->GetExtent();
        glfwWaitEvents();
    }

    graphicsData->GetBackend().WaitForDevice();

    if(swapchain == nullptr)
    {
        Console::log("Recreating swap chain", "Graphics");
        swapchain = std::make_unique<Swapchain>(
            graphicsData->GetBackend().GetDevice(),
            *graphicsData->GetWindow(),
            SwapchainSettings{}
        );
    }
    else
    {
        std::shared_ptr<Swapchain> oldSwapchain = std::move(swapchain);
        Console::log("Resizing swap chain", "FrameOrchestrator");
        swapchain = std::make_unique<Swapchain>(
            graphicsData->GetBackend().GetDevice(),
            *graphicsData->GetWindow(),
            SwapchainSettings{},
            oldSwapchain->GetSwapchain()
        );

        if(!oldSwapchain->CompareSwapFormats(*swapchain.get()))
        {
            // TODO: Handle this better
            throw std::runtime_error("Swap chain image or depth format has changed!");
        }
    }
}

void FrameOrchestrator::createCommandBuffers()
{
    #ifdef DEBUG
    Console::debug("Creating command buffers...", "FrameOrchestrator");
    #endif

    commandBuffers.resize(Swapchain::MAX_FRAMES_IN_FLIGHT);
    // currentFrameIndex = 0;

    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = device.GetCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    VK_CHECK(device.Get().allocateCommandBuffers(&allocInfo, commandBuffers.data()), "Failed to allocate command buffers");
}


void FrameOrchestrator::transitionToPresent(vk::CommandBuffer commandBuffer, vk::Image image)
{
    Image::TransitionVkImageLayout(
        image, 
        vk::ImageLayout::eColorAttachmentOptimal,
        vk::ImageLayout::ePresentSrcKHR,
        commandBuffer,
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    );
}

void FrameOrchestrator::transitionToRenderTarget(vk::CommandBuffer commandBuffer, vk::Image image)
{
    Image::TransitionVkImageLayout(
        image, 
        vk::ImageLayout::ePresentSrcKHR,
        vk::ImageLayout::eColorAttachmentOptimal,
        commandBuffer,
        {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    );
}

} // namespace graphics