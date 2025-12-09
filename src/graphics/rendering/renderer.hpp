#pragma once
#include <memory>
#include "graphics/backend/device.hpp"
#include "swap_chain.hpp"

namespace graphics
{
class Graphics;

struct RenderContext
{
    uint32_t frameIndex;
    double frameTime;
    VkCommandBuffer commandBuffer;
    VkDescriptorSet globalDescriptorSet;
    VkDescriptorSet cameraDescriptorSet;
};

class Renderer
{
public:
    Renderer();
    Renderer(internal::Device &device, Window &window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void RecreateSwapchain();

    VkCommandBuffer BeginFrame();
    void BeginRenderDynamic(VkCommandBuffer cmdBuffer, VkImageView colorView, VkImageView depthView, VkExtent2D extent, VkClearValue clearColor);
    void ExecuteRenderGraph(/*const &RenderGraph graph*/);
    void EndRenderDynamic(VkCommandBuffer cmdBuffer);
    void EndFrame();

    // Getters

    VkExtent2D GetExtent() const { return swapchain->GetSwapChainExtent(); }
    bool IsFrameInProgress() const { return frameInProgress; }
    VkCommandBuffer GetCurrentCommandBuffer() const 
    {
        if(!frameInProgress)
        {
            throw std::runtime_error("Cannot get command buffer as frame is not in progress");
        }
        return commandBuffers[currentFrameIndex]; 
    }
    uint32_t GetFrameIndex() const 
    { 
        if(!frameInProgress)
        {
            throw std::runtime_error("Cannot get frame index as frame is not in progress");
        }
        return currentFrameIndex; 
    }
    const Swapchain &GetSwapchain() const { return *swapchain; }

private:
    internal::Device &device;
    Window &window;

    std::unique_ptr<Swapchain> swapchain{};
    std::vector<VkCommandBuffer> commandBuffers{};
    VkCommandBuffer currentCommandBuffer{};

    uint32_t currentImageIndex = 0;
    uint32_t currentFrameIndex = 0;
    bool frameInProgress = false;
    
    void init();

    void recordCommandBuffer(int imageIndex);
    void createCommandBuffers();
    void recreateSwapChain();

    friend class Graphics;
};
} // namespace graphics