#pragma once
#include <memory>
#include "graphics/backend/device.hpp"
#include "swap_chain.hpp"
#include "render_context.hpp"

namespace graphics
{
class Graphics;

class FrameOrchestrator
{
public:
    FrameOrchestrator();
    FrameOrchestrator(internal::Device &device, Window &window);
    ~FrameOrchestrator();

    FrameOrchestrator(const FrameOrchestrator&) = delete;
    FrameOrchestrator& operator=(const FrameOrchestrator&) = delete;

    void Cleanup();

    void RecreateSwapchain();

    [[nodiscard]] FrameContext BeginFrame();
    // void BeginRenderDynamic(vk::ImageView colorView, vk::ImageView depthView, vk::Extent2D extent, vk::ClearValue clearColor);
    void BeginRenderDynamic(const FrameContext& context, const PassInfo& passInfo);
    void ExecuteRenderGraph(/*const &RenderGraph graph*/);
    void EndRenderDynamic(const FrameContext& context, const PassInfo& passInfo); // Takes pass info, but might not need it
    void EndFrame();

    // Getters

    [[nodiscard]] vk::Extent2D GetExtent() const { return swapchain->GetSwapChainExtent(); }
    [[nodiscard]] bool IsFrameInProgress() const { return frameInProgress; }
    [[nodiscard]] vk::CommandBuffer GetCurrentCommandBuffer() const 
    {
        if(!frameInProgress)
        {
            throw std::runtime_error("Cannot get command buffer as frame is not in progress");
        }
        return commandBuffers[currentFrameIndex]; 
    }
    [[nodiscard]] uint32_t GetFrameIndex() const 
    { 
        if(!frameInProgress)
        {
            throw std::runtime_error("Cannot get frame index as frame is not in progress");
        }
        return currentFrameIndex; 
    }
    [[nodiscard]] uint32_t GetImageIndex() const { return currentImageIndex; }
    [[nodiscard]] const Swapchain &GetSwapchain() const { return *swapchain; }
    [[nodiscard]] FrameContext GetContext() const 
    { 
        if(!frameInProgress)
        {
            throw std::runtime_error("Cannot get render context as no frame is in progress");
        }
        return {currentFrameIndex, currentImageIndex, currentCommandBuffer}; 
    }
private:
    internal::Device &device;
    Window &window;

    std::unique_ptr<Swapchain> swapchain{};
    std::vector<vk::CommandBuffer> commandBuffers{};
    vk::CommandBuffer currentCommandBuffer{};

    uint32_t currentImageIndex = 0;
    uint32_t currentFrameIndex = 0;
    bool frameInProgress = false;
    
    void init();

    void recordCommandBuffer(int imageIndex);
    void createCommandBuffers();
    void recreateSwapChain();

    void transitionToPresent(vk::CommandBuffer commandBuffer, vk::Image image);
    void transitionToRenderTarget(vk::CommandBuffer commandBuffer, vk::Image image);

    friend class Graphics;
};
} // namespace graphics