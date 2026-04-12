#pragma once
#include "graphics/backend/device.hpp"

namespace graphics
{
struct SwapchainSettings
{
    /// @brief Requires the swapchain to use a mode with V-Sync
    bool requireVSync = true;
    /// @brief Allow the use of Mailbox present mode if available
    bool allowMailboxPresentMode = true;
    VkPresentModeKHR overridePresentMode = VK_PRESENT_MODE_MAX_ENUM_KHR; // Force a specific present mode if not MAX_ENUM. Mostly for testing.
};

class Swapchain
{
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    Swapchain(internal::Device &device, Window &window, SwapchainSettings settings = SwapchainSettings{}, vk::SwapchainKHR oldSwapchain = VK_NULL_HANDLE);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    const vk::SwapchainKHR &GetSwapchain() const { return swapchain; }
    vk::Image &GetImage(uint32_t index) { return swapchainImages[index]; }
    const vk::ImageView &GetImageView(uint32_t index) const { return swapchainImageViews[index]; }
    const vk::ImageView &GetDepthImageView(uint32_t index) const { return depthImageViews[index]; }
    const vk::Extent2D GetSwapChainExtent() const { return swapchainExtent; }
    inline uint32_t GetWidth() const { return swapchainExtent.width; }
    inline uint32_t GetHeight() const { return swapchainExtent.height; }
    inline uint32_t GetImageCount() const { return swapchainImages.size(); }
    inline const vk::Format &GetImageFormat() const { return swapchainImageFormat; }
    inline const vk::Format &GetDepthFormat() const { return swapchainDepthFormat; }
    float GetAspectRatio() 
    {
        return static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height);
    }
    inline uint32_t GetCurrentFrame() const { return currentFrame; }

    vk::Result AcquireNextImage(uint32_t *imageIndex);
    vk::Result SubmitCommandBuffers(const std::span<vk::CommandBuffer> buffers, uint32_t* imageIndex);

    bool CompareSwapFormats(const Swapchain& swapchain) const
    {
        return swapchain.swapchainDepthFormat == swapchainDepthFormat && swapchain.swapchainImageFormat == swapchainImageFormat;
    }
private:
    internal::Device &device;
    Window &window;

    SwapchainSettings settings{};

    vk::SwapchainKHR swapchain = VK_NULL_HANDLE;

    // Resources

    std::vector<vk::Image> swapchainImages{};
    std::vector<vk::ImageView> swapchainImageViews{};
    std::vector<vk::Image> depthImages;
    std::vector<VmaAllocation> depthImageAllocations;
    std::vector<VmaAllocationInfo> depthImageAllocationInfos;
    std::vector<vk::ImageView> depthImageViews;

    // Swapchain properties

    vk::Format swapchainImageFormat;
    vk::Format swapchainDepthFormat;
    vk::Extent2D swapchainExtent{};

    // Synchronization objects

    std::vector<vk::Semaphore> imageAvailableSemaphores{};
    std::vector<vk::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::Fence> inFlightFences{};
    std::vector<vk::Fence> imagesInFlight{};
    size_t currentFrame = 0;


    void createSwapchain(vk::SwapchainKHR oldSwapchain = VK_NULL_HANDLE);
    void createImageViews();
    void createDepthImages();
    void createSyncObjects();

    // Might implement to support non-dynamic rendering
    // void createRenderPass();
    // void createFramebuffers();

    // Helper functions
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::set<vk::PresentModeKHR> &availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities);
    vk::Format findDepthFormat();
};
} // namespace graphics::internal