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

    Swapchain(internal::Device &device, Window &window, SwapchainSettings settings = SwapchainSettings{}, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    const VkSwapchainKHR &GetSwapchain() const { return swapchain; }
    VkImage &GetImage(uint32_t index) { return swapchainImages[index]; }
    const VkImageView &GetImageView(uint32_t index) const { return swapchainImageViews[index]; }
    const VkImageView &GetDepthImageView(uint32_t index) const { return depthImageViews[index]; }
    const VkExtent2D GetSwapChainExtent() const { return swapchainExtent; }
    inline uint32_t GetWidth() const { return swapchainExtent.width; }
    inline uint32_t GetHeight() const { return swapchainExtent.height; }
    inline uint32_t GetImageCount() const { return swapchainImages.size(); }
    inline const VkFormat &GetImageFormat() const { return swapchainImageFormat; }
    inline const VkFormat &GetDepthFormat() const { return swapchainDepthFormat; }
    float GetAspectRatio() 
    {
        return static_cast<float>(swapchainExtent.width) / static_cast<float>(swapchainExtent.height);
    }
    inline uint32_t GetCurrentFrame() const { return currentFrame; }

    VkResult AcquireNextImage(uint32_t *imageIndex);
    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    bool CompareSwapFormats(const Swapchain& swapchain) const
    {
        return swapchain.swapchainDepthFormat == swapchainDepthFormat && swapchain.swapchainImageFormat == swapchainImageFormat;
    }
private:
    internal::Device &device;
    Window &window;

    SwapchainSettings settings{};

    VkSwapchainKHR swapchain = VK_NULL_HANDLE;

    // Resources

    std::vector<VkImage> swapchainImages{};
    std::vector<VkImageView> swapchainImageViews{};
    std::vector<VkImage> depthImages;
    std::vector<VmaAllocation> depthImageAllocations;
    std::vector<VmaAllocationInfo> depthImageAllocationInfos;
    std::vector<VkImageView> depthImageViews;

    // Swapchain properties

    VkFormat swapchainImageFormat;
    VkFormat swapchainDepthFormat;
    VkExtent2D swapchainExtent{};

    // Synchronization objects

    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    size_t currentFrame = 0;


    void createSwapchain(VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
    void createImageViews();
    void createDepthImages();
    void createSyncObjects();

    // Might implement to support non-dynamic rendering
    // void createRenderPass();
    // void createFramebuffers();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::set<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    VkFormat findDepthFormat();
};
} // namespace graphics::internal