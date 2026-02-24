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
        graphicsData->pipelineManager.init();

        graphicsData->pipelineManager.CreatePipelines();
        
        // testImage = std::make_unique<Image>(
        //     graphicsData->GetBackend().GetDevice(),
        //     512,
        //     512,
        //     graphics::ImageProperties::getDefaultProperties(),
        //     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        // );
        // testImage->TransitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // tests::RunAllTests();
        graphicsData->cameraSetLayout = DescriptorSetLayout::Builder()
            .AddBinding(
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
            )
            .Build();
        graphicsData->globalSetLayout = DescriptorSetLayout::Builder()
            .AddBinding(
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
            )
            .Build();

        Console::log("Graphics module initialized", "Graphics");
        


        graphicsData->globalDescriptorPool = DescriptorPool::Builder(graphicsData->GetBackend().GetDevice())
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
            .SetPoolFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
            .SetMaxSets(2)
            .Build();
        graphicsData->cameraDescriptorPool = DescriptorPool::Builder(graphicsData->GetBackend().GetDevice())
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .SetPoolFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
            .SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
            .Build();
        // ShaderLayout s{};
        // std::vector<ShaderLayout> layouts = {s};
        // graphicsData->globalDescriptorBuffer = std::make_unique<DescriptorBuffer>(
        //     graphicsData->GetBackend().GetDevice(),
        //     layouts,
        //     20
        // );
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
            uint32_t imageIndex = renderer.GetImageIndex();
            renderer.BeginRenderDynamic(
                commandBuffer,
                renderer.GetSwapchain().GetImageView(imageIndex),
                renderer.GetSwapchain().GetDepthImageView(imageIndex),
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