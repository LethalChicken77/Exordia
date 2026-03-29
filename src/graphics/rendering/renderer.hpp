#pragma once
#include <memory>
#include "graphics/backend/device.hpp"
#include "swap_chain.hpp"
#include "render_context.hpp"

namespace graphics
{
class Graphics;

class Renderer
{
public:
    Renderer();
    Renderer(internal::Device &device, Window &window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Cleanup();

    void RecreateSwapchain();

    [[nodiscard]] VkCommandBuffer BeginFrame();
    void BeginRenderDynamic(VkCommandBuffer cmdBuffer, VkImageView colorView, VkImageView depthView, VkExtent2D extent, VkClearValue clearColor);
    void ExecuteRenderGraph(/*const &RenderGraph graph*/);
    void EndRenderDynamic(VkCommandBuffer cmdBuffer);
    void EndFrame();

    // Getters

    [[nodiscard]] VkExtent2D GetExtent() const { return swapchain->GetSwapChainExtent(); }
    [[nodiscard]] bool IsFrameInProgress() const { return frameInProgress; }
    [[nodiscard]] VkCommandBuffer GetCurrentCommandBuffer() const 
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
    [[nodiscard]] RenderContext GetContext() const 
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
    std::vector<VkCommandBuffer> commandBuffers{};
    VkCommandBuffer currentCommandBuffer{};

    std::vector<bool> firstUse{};

    uint32_t currentImageIndex = 0;
    uint32_t currentFrameIndex = 0;
    bool frameInProgress = false;
    
    void init();

    void recordCommandBuffer(int imageIndex);
    void createCommandBuffers();
    void recreateSwapChain();

    void transitionToPresent(VkCommandBuffer commandBuffer, VkImage image);
    void transitionToRenderTarget(VkCommandBuffer commandBuffer, VkImage image);

    friend class Graphics;
};
} // namespace graphics