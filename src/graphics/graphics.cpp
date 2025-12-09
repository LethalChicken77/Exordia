#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"
#include "backend/buffer.hpp"
#include "backend/image.hpp"
#include "rendering/swap_chain.hpp"

#include "tests/graphics_tests.hpp"

namespace graphics
{
    // std::unique_ptr<Image> testImage;
    std::unique_ptr<Swapchain> testSwapchain{};

    void recreateSwapChain()
    {
        VkExtent2D extent = graphicsData->GetWindow().GetExtent();
        while(extent.width == 0 || extent.height == 0)
        {
            extent = graphicsData->GetWindow().GetExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(graphicsData->GetBackend().GetDevice().GetDevice());

        if(testSwapchain == nullptr)
        {
            Console::log("Recreating swap chain", "Graphics");
            testSwapchain = std::make_unique<Swapchain>(
                graphicsData->GetBackend().GetDevice(),
                graphicsData->GetWindow(),
                SwapchainSettings::GetDefaultSettings()
            );
        }
        else
        {
            std::shared_ptr<Swapchain> oldSwapchain = std::move(testSwapchain);
            Console::log("Resizing swap chain", "Graphics");
            testSwapchain = std::make_unique<Swapchain>(
                graphicsData->GetBackend().GetDevice(),
                graphicsData->GetWindow(),
                SwapchainSettings::GetDefaultSettings(),
                oldSwapchain->GetSwapchain()
            );

            if(!oldSwapchain->CompareSwapFormats(*testSwapchain.get()))
            {
                // TODO: Handle this better
                throw std::runtime_error("Swap chain image or depth format has changed!");
            }
        }
        // currentRenderPass = swapChain->getRenderPass();
        // SHeesh
    }

    Graphics::Graphics(const std::string& appName, const std::string& engName)
    {
        Console::log("Initializing graphics module", "Graphics");
        graphicsData = std::make_unique<GraphicsData>();
        graphicsData->window.init(800, 600, engName + " - " + appName);
        graphicsData->backend.init(appName, engName, graphicsData->window);

        // testImage = std::make_unique<Image>(
        //     graphicsData->GetBackend().GetDevice(),
        //     512,
        //     512,
        //     graphics::ImageProperties::getDefaultProperties(),
        //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        // );
        // testImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // tests::RunAllTests();

        Console::log("Graphics module initialized", "Graphics");
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
    }
} // namespace graphics