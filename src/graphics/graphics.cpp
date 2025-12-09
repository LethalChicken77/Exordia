#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"
#include "backend/buffer.hpp"
#include "backend/image.hpp"
#include "rendering/swap_chain.hpp"

#include "tests/graphics_tests.hpp"

namespace graphics
{
    Graphics::Graphics(const std::string& appName, const std::string& engName)
    {
        Console::log("Initializing graphics module", "Graphics");
        graphicsData = std::make_unique<GraphicsData>();
        graphicsData->window.init(800, 600, engName + " - " + appName);
        graphicsData->backend.init(appName, engName, graphicsData->window);
        graphicsData->renderer.init();

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

    void Graphics::DrawFrame()
    {
        Renderer &renderer = graphicsData->renderer;
        VkExtent2D extent = renderer.GetExtent();
        if(extent.width <= 0 || extent.height <= 0) return; // Don't draw frame if minimized

        std::vector<VkDescriptorSet> localDescriptorSets;
        if(VkCommandBuffer commandBuffer = renderer.BeginFrame())
        {
            uint32_t frameIndex = renderer.GetFrameIndex();
            renderer.BeginRenderDynamic(
                commandBuffer,
                renderer.GetSwapchain().GetImageView(frameIndex),
                renderer.GetSwapchain().GetDepthImageView(frameIndex),
                extent,
                VkClearValue{.color = {{0.02f, 0.03f, 0.1f, 1.0f}}}
            );
            renderer.EndRenderDynamic(commandBuffer);
            // RenderContext frameInfo{frameIndex, 0.0, commandBuffer, Descriptors::globalDescriptorSet, Descriptors::cameraDescriptorSets[frameIndex]};
            renderer.EndFrame();
        }
        graphicsData->GetBackend().WaitForDevice();
        // sceneRenderQueue.clear();
        // outlineRenderQueue.clear();
    }
} // namespace graphics