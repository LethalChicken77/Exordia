#include "utils/debug.hpp"
#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"
#include "backend/buffer.hpp"
#include "backend/image.hpp"
#include "rendering/swap_chain.hpp"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "rendering/draw_funcs.hpp"

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
        graphicsData->materialDescriptorPool = DescriptorPool::Builder(graphicsData->GetBackend().GetDevice())
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .SetPoolFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
            .SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
            .Build();
        // BufferLayout s{};
        // std::vector<BufferLayout> layouts = {s};
        // graphicsData->globalDescriptorBuffer = std::make_unique<DescriptorBuffer>(
        //     graphicsData->GetBackend().GetDevice(),
        //     layouts,
        //     20
        // );

        // Init global UBO
        graphicsData->globalSetLayout = DescriptorSetLayout::Builder()
            .AddBinding(
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
            )
            .Build();
        graphicsData->globalUBO = std::make_unique<Buffer>(
            graphicsData->GetBackend().GetDevice(),
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            graphicsData->GetDeviceProperties().properties.limits.minUniformBufferOffsetAlignment
        );
        graphicsData->globalUBO->Map();
        graphicsData->globalSetLayout = DescriptorSetLayout::Builder(graphicsData->GetDevice())
            .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
        VkDescriptorBufferInfo bufferInfo = graphicsData->globalUBO->GetDescriptorInfo();
        DescriptorWriter(*graphicsData->globalSetLayout, *graphicsData->globalDescriptorPool)
            .WriteBuffer(0, &bufferInfo)
            .Build(graphicsData->globalDescriptorSet);

        // Init camera UBO
        Console::log("Creating camera UBO", "Graphics");
        graphicsData->cameraUBOs.clear();
        for(int i = 0; i < Swapchain::MAX_FRAMES_IN_FLIGHT; i++)
        {
            graphicsData->cameraUBOs.emplace_back(Buffer(
                graphicsData->GetDevice(), 
                sizeof(CameraUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                graphicsData->GetDeviceProperties().properties.limits.minUniformBufferOffsetAlignment
            ));
            graphicsData->cameraUBOs[i].Map();
        }
        graphicsData->cameraSetLayout = DescriptorSetLayout::Builder()
            .AddBinding(
                0,
                VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT
            )
            .Build();
        
        graphicsData->cameraDescriptorSets = std::vector<VkDescriptorSet>(Swapchain::MAX_FRAMES_IN_FLIGHT);
        for(int i = 0; i < graphicsData->cameraDescriptorSets.size(); i++)
        {
            VkDescriptorBufferInfo bufferInfo = graphicsData->cameraUBOs[i].GetDescriptorInfo();
            DescriptorWriter(*graphicsData->cameraSetLayout, *graphicsData->cameraDescriptorPool)
                .WriteBuffer(0, &bufferInfo)
                .Build(graphicsData->cameraDescriptorSets[i]);
        }
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
        drawQueue.clear();
        graphicsData.reset();
    }

    void Graphics::DrawMesh(const core::Mesh& meshData, id_t materialID, const glm::mat4& modelMatrix, int instanceID)
    {
        drawQueue.push_back(MeshRenderData(meshData->graphicsHandle, modelMatrix, materialID, instanceID));
    }

    void Graphics::DrawFrame()
    {
        Renderer &renderer = graphicsData->renderer;
        VkExtent2D extent = renderer.GetExtent();
        if(extent.width <= 0 || extent.height <= 0) return; // Don't draw frame if minimized

        std::vector<VkDescriptorSet> localDescriptorSets;
        if(VkCommandBuffer commandBuffer = renderer.BeginFrame())
        {
            RenderContext renderContext = renderer.GetContext();
            renderer.BeginRenderDynamic(
                commandBuffer,
                renderer.GetSwapchain().GetImageView(renderContext.imageIndex),
                renderer.GetSwapchain().GetDepthImageView(renderContext.imageIndex),
                extent,
                VkClearValue{.color = {{0.02f, 0.03f, 0.1f, 1.0f}}}
            );
            if(drawQueue.size() > 0)
            {
                GraphicsPipeline &currentPipeline = *graphicsData->pipelineManager.GetPipeline(0);
                currentPipeline.Bind(commandBuffer);
                localDescriptorSets.push_back(graphicsData->testMaterial->GetDescriptorSet());
    
                DrawFunctions::bindCameraDescriptor(renderContext, graphicsData->cameraDescriptorSets[renderContext.frameIndex], &currentPipeline);
                DrawFunctions::bindGlobalDescriptor(renderContext, graphicsData->globalDescriptorSet, &currentPipeline);
                vkCmdBindDescriptorSets(
                    commandBuffer, 
                    VK_PIPELINE_BIND_POINT_GRAPHICS, 
                    currentPipeline.GetPipelineLayout(), 
                    2,
                    1,
                    localDescriptorSets.data(), 
                    0,
                    nullptr
                );

                const GraphicsMesh* mesh = graphicsData->meshRegistry.Get(drawQueue[0].handle);
                if(mesh != nullptr)
                {
                    mesh->bind(commandBuffer, drawQueue[0].instanceBuffer);
                    mesh->draw(commandBuffer, 1);
                }
    
                
            }

            renderer.EndRenderDynamic(commandBuffer);
            drawImgui(renderContext);
            // RenderContext frameInfo{frameIndex, 0.0, commandBuffer, Descriptors::globalDescriptorSet, Descriptors::cameraDescriptorSets[frameIndex]};
            renderer.EndFrame();
        }
        graphicsData->GetBackend().WaitForDevice();
        // sceneRenderQueue.clear();
        // outlineRenderQueue.clear();
        drawQueue.clear();
    }

    void Graphics::GraphicsInitImgui()
    {
        // std::cout << "Configuring IMGUI" << std::endl;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& imguiIO = ImGui::GetIO();
        (void)imguiIO;
        // Set style (optional)
        // ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();

        // Set window rounding
        // style.WindowRounding = 5.0f;
        // style.FrameRounding = 3.0f;
        // style.GrabRounding = 2.0f;

        // Adjust padding and spacing
        // style.WindowPadding = ImVec2(10, 10);
        // style.FramePadding = ImVec2(5, 5);
        // style.ItemSpacing = ImVec2(8, 4);

        // Modify colors

        ImGui_ImplGlfw_InitForVulkan(GetGLFWWindow(), true);

        internal::Device &device = GetDevice();
        const Swapchain &swapchain = graphicsData->renderer.GetSwapchain();
        // containers.imguiDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
        graphicsData->imguiDescriptorPool = DescriptorPool::Builder(device)
            .SetMaxSets(Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, Swapchain::MAX_FRAMES_IN_FLIGHT)
            .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .Build();

        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = graphicsData->backend.GetInstance();
        initInfo.PhysicalDevice = device.GetPhysicalDevice().Get();
        initInfo.Device = device.Get();
        initInfo.QueueFamily = device.GetPhysicalDevice().GetQueueFamilyIndices().graphicsFamily;
        initInfo.Queue = device.GetGraphicsQueue();
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = graphicsData->imguiDescriptorPool->GetPool();
        initInfo.Allocator = nullptr;
        initInfo.MinImageCount = 2;
        initInfo.ImageCount = Swapchain::MAX_FRAMES_IN_FLIGHT;

        initInfo.UseDynamicRendering = true;

        initInfo.PipelineInfoMain = {};
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = {};
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain.GetImageFormat();
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = swapchain.GetDepthFormat();
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        // For multisampling, probably won't use
        //initInfo.PipelineRenderingCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        
        initInfo.PipelineInfoForViewports = initInfo.PipelineInfoMain;

        initInfo.CheckVkResultFn = [](VkResult err) {
            if (err != VK_SUCCESS) {
                fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
                Console::errorf("Vulkan Error: {}", Debug::VkResultToString(err), "[ImGui]");
            }
        };
        ImGui_ImplVulkan_Init(&initInfo);

        // std::cout << viewportTexture->getSampler() << std::endl;
        // viewportDescriptorSet = ImGui_ImplVulkan_AddTexture(
        //     viewportTexture->getSampler(),
        //     viewportTexture->getImageView(),
        //     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        // );
    }

    void Graphics::drawImgui(RenderContext context)
    {
        VkRenderingAttachmentInfo colorAttachment = {};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = graphicsData->renderer.GetSwapchain().GetImageView(context.imageIndex);
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo renderingInfo = {};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea = {{0,0}, graphicsData->renderer.GetExtent()};
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;

        vkCmdBeginRendering(context.commandBuffer, &renderingInfo);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), context.commandBuffer);
        vkCmdEndRendering(context.commandBuffer);
    }
} // namespace graphics