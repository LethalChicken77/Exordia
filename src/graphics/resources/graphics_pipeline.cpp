#include <string>
#include <iostream>
#include <vector>

#include "graphics_pipeline.hpp"
#include "utils/debug.hpp"
#include "graphics/resources/graphics_mesh.hpp"

#include "graphics/push_constants.hpp"
#include "graphics/graphics_data.hpp"
#include "console.hpp"

using namespace std;

namespace graphics
{
    /// @brief Create a graphics pipeline with default configuration
    /// @param device Reference to the logical device
    /// @param id ID assigned by the pipeline manager
    /// @param shader Shader to use
    /// @param cache Pipeline cache to use
    GraphicsPipelineOld::GraphicsPipelineOld(internal::Device &_device, id_t id, const Shader &shader, VkPipelineCache cache) 
        : device(_device), ID(id), configInfo(shader.properties), materialSetLayout(createDescriptorSetLayout(shader))
    {
        createShaderModules(shader);
        switch(configInfo.pipelineType)
        {
            case PipelineType::Standard:
                createStandardLayout();
                createStandardPipeline(cache);
                break;
            // case PipelineType::POST_PROCESSING:
            //     createPostProcessingLayout();
            //     createPostProcessingPipeline(cache);
            //     break;
            // case PipelineType::ID_BUFFER:
            //     createIDBufferLayout();
            //     createIDBufferPipeline(cache);
            //     break;
            default:
                Console::error("Unrecognized pipeline type: " + std::to_string((int)configInfo.pipelineType), "Graphics Pipeline");
        }
    }

    GraphicsPipelineOld::~GraphicsPipelineOld()
    {
        if(graphicsPipeline != nullptr)
            vkDestroyPipeline(device.Get(), graphicsPipeline, nullptr);
        if(pipelineLayout != nullptr)
            vkDestroyPipelineLayout(device.Get(), pipelineLayout, nullptr);
        if(vertexShaderModule != nullptr)
            vkDestroyShaderModule(device.Get(), vertexShaderModule, nullptr);
        if(fragmentShaderModule != nullptr)
            vkDestroyShaderModule(device.Get(), fragmentShaderModule, nullptr);
    }

    void GraphicsPipelineOld::createShaderModules(const Shader &shader)
    {
        const std::vector<uint32_t> &vertSpv = shader.GetVertSpirv();
        const std::vector<uint32_t> &fragSpv = shader.GetFragSpirv();
        if(vertSpv.empty())
        {
            Console::error("Cannot initialize graphics pipeline: Vertex shader SPIR-V is empty", "GraphicsPipeline");
            return;
        }
        if(fragSpv.empty())
        {
            Console::error("Cannot initialize graphics pipeline: Fragment shader SPIR-V is empty", "GraphicsPipeline");
            return;
        }
        createShaderModule(vertSpv, &vertexShaderModule);
        createShaderModule(fragSpv, &fragmentShaderModule);
    }

    void GraphicsPipelineOld::createShaderModule(const std::vector<uint32_t>& spvCode, VkShaderModule* shaderModule)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spvCode.size() * sizeof(uint32_t);
        createInfo.pCode = spvCode.data();

        VK_CHECK(vkCreateShaderModule(device.Get(), &createInfo, nullptr, shaderModule), "Failed to create shader module");
    }

    using BindingType = ShaderLayout::BindingType;
    DescriptorSetLayout GraphicsPipelineOld::createDescriptorSetLayout(const Shader &shader)
    {        
        DescriptorSetLayout::Builder builder{device};
        const ShaderLayout &layout = shader.GetLayout();
        const ShaderLayout::DescriptorSetInfo *setInfo = shader.GetLayout().GetMaterialDescriptorSet();
        if(setInfo == nullptr)
            return builder.BuildInPlace();
        for(const ShaderLayout::BindingInfo &bindingInfo : setInfo->bindings)
        {
            VkDescriptorType type{};
            switch(bindingInfo.type)
            {
                case BindingType::Sampler:
                    type = VK_DESCRIPTOR_TYPE_SAMPLER;
                    break;
                case BindingType::CombinedImageSampler:
                    type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    break;
                case BindingType::SampledImage:
                    type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                    break;
                case BindingType::StorageImage:
                    type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    break;
                case BindingType::InputAttachment:
                    type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
                    break;
                case BindingType::UniformTexelBuffer:
                    type = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
                    break;
                case BindingType::StorageTexelBuffer:
                    type = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
                    break;
                case BindingType::UniformBuffer:
                    type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;
                case BindingType::StorageBuffer:
                    type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    break;
                case BindingType::DynamicUniformBuffer:
                    type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
                    break;
                case BindingType::DynamicStorageBuffer:
                    type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
                    break;
                case BindingType::AccelerationStructure:
                    type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
                    break;
                case BindingType::InlineUniformBlock:
                    type = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK;
                    break;
                case BindingType::Invalid:
                default:
                    type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
                    break;
            }
            builder.AddBinding(bindingInfo.binding, type, bindingInfo.stageFlags, bindingInfo.count);
        }
        return builder.BuildInPlace();
    }
    
    void GraphicsPipelineOld::createStandardLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            graphicsData->cameraSetLayout->GetDescriptorSetLayout(),
            graphicsData->globalSetLayout->GetDescriptorSetLayout(),
            materialSetLayout.GetDescriptorSetLayout()
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(device.Get(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create standard pipeline layout");
        }
    }

    // void GraphicsPipeline::createPostProcessingLayout()
    // {
    //     Console::log("Creating post processing pipeline layout", "GraphicsPipeline");

    //     std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
    //         shader.getDescriptorSetLayout()->getDescriptorSetLayout() // Put post-processing data in here
    //     };

    //     VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    //     pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //     pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    //     pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    //     pipelineLayoutInfo.pushConstantRangeCount = 0;
    //     pipelineLayoutInfo.pPushConstantRanges = nullptr;
    //     if(vkCreatePipelineLayout(Shared::device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("Failed to create post processing pipeline layout");
    //     }
    // }
    
    // void GraphicsPipeline::createIDBufferLayout()
    // {
    //     Console::log("Creating ID buffer pipeline layout", "GraphicsPipeline");

    //     VkPushConstantRange pushConstantRange{};
    //     pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    //     pushConstantRange.offset = 0;
    //     pushConstantRange.size = sizeof(PushConstants);

    //     std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
    //         Descriptors::cameraSetLayout->getDescriptorSetLayout(),
    //         shader.getDescriptorSetLayout()->getDescriptorSetLayout()
    //     };

    //     VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    //     pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //     pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    //     pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    //     pipelineLayoutInfo.pushConstantRangeCount = 1;
    //     pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    //     if(vkCreatePipelineLayout(device.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("Failed to create ID buffer pipeline layout");
    //     }
    // }

    void GraphicsPipelineOld::createStandardPipeline(VkPipelineCache cache)
    {
        assert(pipelineLayout != nullptr && "Cannot create graphics pipeline:: layout is null");

        configInfo.colorAttachmentFormats = { vk::Format::eB8G8R8A8Srgb };
        configInfo.depthAttachmentFormat = vk::Format::eD32Sfloat;

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertexShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragmentShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions = GraphicsMesh::getVertexBindingDescriptions();
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GraphicsMesh::getVertexAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkPipelineDynamicStateCreateInfo dynamicStateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size()),
            .pDynamicStates = dynamicStateEnables.data(),
        };

        // VkPipelineCreateFlags2CreateInfo flags2{};
        // flags2.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO;
        // flags2.pNext = nullptr;
        // flags2.flags = 0;
        std::vector<VkFormat> convertedFormats; // Temporary conversion until new pipelines are ready
        convertedFormats.reserve(configInfo.colorAttachmentFormats.size());
        for (vk::Format f : configInfo.colorAttachmentFormats)
        {
            convertedFormats.push_back(static_cast<VkFormat>(f));
        }

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            // .pNext = &flags2, // Remove for classic descriptor sets
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<uint32_t>(configInfo.colorAttachmentFormats.size()),
            .pColorAttachmentFormats = convertedFormats.data(),
            .depthAttachmentFormat = static_cast<VkFormat>(configInfo.depthAttachmentFormat),
            .stencilAttachmentFormat = static_cast<VkFormat>(configInfo.stencilAttachmentFormat)
        };

        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = (VkPipelineInputAssemblyStateCreateInfo*)&configInfo.inputAssemblyInfo,
            .pViewportState = (VkPipelineViewportStateCreateInfo*)&configInfo.viewportInfo,
            .pRasterizationState = (VkPipelineRasterizationStateCreateInfo*)&configInfo.rasterizationInfo,
            .pMultisampleState = (VkPipelineMultisampleStateCreateInfo*)&configInfo.multisampleInfo,
            .pDepthStencilState = (VkPipelineDepthStencilStateCreateInfo*)&configInfo.depthStencilInfo,
            .pColorBlendState = (VkPipelineColorBlendStateCreateInfo*)&configInfo.colorBlendInfo,
            .pDynamicState = (VkPipelineDynamicStateCreateInfo*)&dynamicStateInfo,

            .layout = pipelineLayout,
            .renderPass = nullptr,
            .subpass = 0,

            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        if(vkCreateGraphicsPipelines(device.Get(), cache, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create standard pipeline! (Skill issue)");
        }
        Console::log("Created standard pipeline successfully", "GraphicsPipeline");
    }

    // void GraphicsPipeline::createPostProcessingPipeline(VkPipelineCache cache)
    // {
    //     Console::log("Creating post processing pipeline", "GraphicsPipeline");
    //     shader.parentPipeline = this;
    //     PipelineConfigInfo &configInfo = shader.getConfigInfo();
        
    //     assert(pipelineLayout != nullptr && "Cannot create graphics pipeline:: layout is null");

    //     VkPipelineShaderStageCreateInfo shaderStages[2];
    //     shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //     shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    //     shaderStages[0].module = shader.getVertexModule();
    //     shaderStages[0].pName = "main";
    //     shaderStages[0].flags = 0;
    //     shaderStages[0].pNext = nullptr;
    //     shaderStages[0].pSpecializationInfo = nullptr;

    //     shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //     shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    //     shaderStages[1].module = shader.getFragmentModule();
    //     shaderStages[1].pName = "main";
    //     shaderStages[1].flags = 0;
    //     shaderStages[1].pNext = nullptr;
    //     shaderStages[1].pSpecializationInfo = nullptr;

    //     VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    //     vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //     vertexInputInfo.vertexAttributeDescriptionCount = 0;
    //     vertexInputInfo.vertexBindingDescriptionCount = 0;
    //     vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    //     vertexInputInfo.pVertexBindingDescriptions = nullptr; // Vertices are defined in shader as screen-space quad

    //     VkGraphicsPipelineCreateInfo pipelineInfo{};
    //     pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //     pipelineInfo.stageCount = 2;
    //     pipelineInfo.pStages = shaderStages;
    //     pipelineInfo.pVertexInputState = &vertexInputInfo;
    //     pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    //     pipelineInfo.pViewportState = &configInfo.viewportInfo;
    //     pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    //     pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    //     pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    //     pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    //     pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

    //     pipelineInfo.layout = pipelineLayout;
    //     pipelineInfo.renderPass = *configInfo.renderPass;
    //     pipelineInfo.subpass = configInfo.subpass;

        
    //     pipelineInfo.basePipelineIndex = -1;
    //     pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    //     if(vkCreateGraphicsPipelines(Shared::device->device(), cache, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("Failed to create post processing pipeline!");
    //     }
    //     // Console::log("Created post processing pipeline successfully", "GraphicsPipeline");
    // }

    // void GraphicsPipeline::createIDBufferPipeline(VkPipelineCache cache)
    // {
    //     Console::log("Creating ID buffer pipeline", "GraphicsPipeline");
    //     PipelineConfigInfo &configInfo = shader.getConfigInfo();
        
    //     assert(pipelineLayout != nullptr && "Cannot create graphics pipeline:: layout is null");
    //     assert(configInfo.renderPass != nullptr && "Cannot create graphics pipeline:: render pass is null");

    //     VkPipelineShaderStageCreateInfo shaderStages[2];
    //     shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //     shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    //     shaderStages[0].module = shader.getVertexModule();
    //     shaderStages[0].pName = "main";
    //     shaderStages[0].flags = 0;
    //     shaderStages[0].pNext = nullptr;
    //     shaderStages[0].pSpecializationInfo = nullptr;

    //     shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    //     shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    //     shaderStages[1].module = shader.getFragmentModule();
    //     shaderStages[1].pName = "main";
    //     shaderStages[1].flags = 0;
    //     shaderStages[1].pNext = nullptr;
    //     shaderStages[1].pSpecializationInfo = nullptr;

    //     std::vector<VkVertexInputBindingDescription> bindingDescriptions = GraphicsMesh::getVertexBindingDescriptions();
    //     std::vector<VkVertexInputAttributeDescription> attributeDescriptions = GraphicsMesh::getVertexAttributeDescriptions();
    //     VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    //     vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    //     vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    //     vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
    //     vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
    //     vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

    //     VkGraphicsPipelineCreateInfo pipelineInfo{};
    //     pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    //     pipelineInfo.stageCount = 2;
    //     pipelineInfo.pStages = shaderStages;
    //     pipelineInfo.pVertexInputState = &vertexInputInfo;
    //     pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
    //     pipelineInfo.pViewportState = &configInfo.viewportInfo;
    //     pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
    //     pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
    //     pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
    //     pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
    //     pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

    //     pipelineInfo.layout = pipelineLayout;
    //     pipelineInfo.renderPass = *configInfo.renderPass;
    //     pipelineInfo.subpass = configInfo.subpass;

        
    //     pipelineInfo.basePipelineIndex = -1;
    //     pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    //     if(vkCreateGraphicsPipelines(Shared::device->device(), cache, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
    //     {
    //         throw std::runtime_error("Failed to create standard pipeline! (Skill issue)");
    //     }
    //     Console::log("Created standard pipeline successfully", "GraphicsPipeline");
    // }

    void GraphicsPipelineOld::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

}