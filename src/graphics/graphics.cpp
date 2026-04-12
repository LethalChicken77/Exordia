#include "utils/debug.hpp"
#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"
#include "resources/buffer.hpp"
#include "resources/image.hpp"
#include "rendering/swap_chain.hpp"
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "rendering/draw_funcs.hpp"
#include "limits.hpp"

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
        graphicsData->pipelineRegistry.init();
        graphicsData->textureRegistry.Init();
        
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
            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
            .SetPoolFlags(VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT)
            .SetMaxSets(EXO_MAX_MATERIALS)
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
            .WriteBuffer(0, bufferInfo)
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
                .WriteBuffer(0, bufferInfo)
                .Build(graphicsData->cameraDescriptorSets[i]);
        }
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
        drawQueue.clear();
        graphicsData.reset();
        graphicsData.release(); // Ensure it's not freed again on exit
    }

    void Graphics::SetCamera(const Camera &camera)
    {
        cameraState.view = camera.getView();
        cameraState.invView = glm::inverse(camera.getView());
        cameraState.proj = camera.getProjection();
        cameraState.viewProj = camera.getViewProjection();
    }

    void Graphics::DrawMesh(const core::Mesh& meshData, const Material& material, const std::vector<glm::mat4>& modelMatrices, int instanceID)
    {
        drawQueue.push_back(MeshRenderData(meshData->graphicsHandle, modelMatrices, material.graphicsHandle, instanceID));
    }

    void Graphics::DrawMesh(const core::Mesh& meshData, const Material& material, const glm::mat4& modelMatrix, int instanceID)
    {
        drawQueue.push_back(MeshRenderData(meshData->graphicsHandle, modelMatrix, material.graphicsHandle, instanceID));
    }

    void Graphics::DrawFrame()
    {
        FrameOrchestrator &renderer = graphicsData->renderer;
        VkExtent2D extent = renderer.GetExtent();
        if(extent.width <= 0 || extent.height <= 0) return; // Don't draw frame if minimized

        std::vector<VkDescriptorSet> localDescriptorSets;
        // VkCommandBuffer commandBuffer; // Attempt to block if swapchain isn't ready yet. This didn't work
        // while (!(commandBuffer = renderer.BeginFrame())) {}
        FrameContext renderContext = renderer.BeginFrame();
        if(renderContext.commandBuffer != nullptr)
        {
            GlobalUbo globalUboData{};
            // globalUboData.lights[0] = {glm::vec3(1, 1, 1), LightType::DIRECTIONAL, glm::vec3(1.0, 1.0, 1.0), 6.0};
            globalUboData.lights[0] = {glm::vec3(1, 1, 1), LightType::DIRECTIONAL, glm::vec3(1.0, 1.0, 1.0), 3.0};
            // globalUboData.lights[0] = {glm::vec3(1, 1, 1), LightType::DIRECTIONAL, glm::vec3(1.0, 1.0, 1.0), 0.0};
            globalUboData.lights[1] = {glm::vec3(4, 0, 0), LightType::POINT, glm::vec3(1.0, 0.8, 0.1), 1000.0};
            globalUboData.lights[2] = {glm::vec3(20, 0, -15), LightType::POINT, glm::vec3(0.5, 1.0, 0.1), 100.0};
            globalUboData.lights[3] = {glm::vec3(-15, 0, 10), LightType::POINT, glm::vec3(0.9, 0.2, 1.0), 100.0};
            globalUboData.numLights = 4;
            globalUboData.ambient = glm::vec3(0.04, 0.08, 0.2);
            // globalUboData.ambient = glm::vec3(1, 1, 1);
            graphicsData->globalUBO->WriteData(&globalUboData);

            graphicsData->cameraUBOs[renderContext.frameIndex].WriteData(&cameraState);

            PassInfo passInfo{};
            passInfo.colorView = renderer.GetSwapchain().GetImageView(renderContext.imageIndex);
            passInfo.depthView = renderer.GetSwapchain().GetDepthImageView(renderContext.imageIndex);
            passInfo.extent = extent;
            passInfo.clearColor = {{0.02f, 0.03f, 0.1f, 1.0f}};
            renderer.BeginRenderDynamic(
                renderContext,
                passInfo
            );

            for(MeshRenderData &renderData : drawQueue)
            {
                GraphicsMaterial *mat = graphicsData->materialRegistry.Get(renderData.materialHandle);
                // Console::debugf("{}", renderData.materialHandle.index);
                if(mat == nullptr) continue;
                GraphicsPipelineOld *currentPipeline = graphicsData->pipelineRegistry.Get(mat->base->shader->graphicsHandle);
                if(currentPipeline == nullptr) continue;
                currentPipeline->Bind(renderContext.commandBuffer);
                localDescriptorSets = {mat->GetDescriptorSet()}; // This is terrible. TODO: literally anything else

                DrawFunctions::bindCameraDescriptor(renderContext, graphicsData->cameraDescriptorSets[renderContext.frameIndex], currentPipeline);
                DrawFunctions::bindGlobalDescriptor(renderContext, graphicsData->globalDescriptorSet, currentPipeline);

                renderContext.commandBuffer.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    vk::PipelineLayout(currentPipeline->GetPipelineLayout()), // Temporary until pipeline rewrite is finished 
                    2,
                    1,
                    (vk::DescriptorSet*)localDescriptorSets.data(), 
                    0,
                    nullptr
                );

                const GraphicsMesh* mesh = graphicsData->meshRegistry.Get(renderData.handle);
                if(mesh != nullptr)
                {
                    mesh->bind(renderContext.commandBuffer, renderData.instanceBuffer);
                    mesh->draw(renderContext.commandBuffer, renderData.transforms.size());
                }
            }

            renderer.EndRenderDynamic(
                renderContext,
                passInfo
            );
            drawImgui(renderContext);
            // FrameContext frameInfo{frameIndex, 0.0, commandBuffer, Descriptors::globalDescriptorSet, Descriptors::cameraDescriptorSets[frameIndex]};
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
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = (VkFormat*)&swapchain.GetImageFormat();
        initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = (VkFormat)swapchain.GetDepthFormat();
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

    void Graphics::drawImgui(FrameContext context)
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