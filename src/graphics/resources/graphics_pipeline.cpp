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
    GraphicsPipeline::GraphicsPipeline(internal::Device &_device, id_t id, const Shader &shader, VkPipelineCache cache) 
        : device(_device), ID(id), configInfo(shader.properties), materialSetLayout(createDescriptorSetLayout(shader))
    {
        createShaderModules(shader);
        switch(configInfo.pipelineType)
        {
            case PipelineType::STANDARD:
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

    GraphicsPipeline::~GraphicsPipeline()
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

    void GraphicsPipeline::createShaderModules(const Shader &shader)
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

    void GraphicsPipeline::createShaderModule(const std::vector<uint32_t>& spvCode, VkShaderModule* shaderModule)
    {
        Console::log("Creating shader module", "ShaderBase");
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = spvCode.size() * sizeof(uint32_t);
        createInfo.pCode = spvCode.data();

        VK_CHECK(vkCreateShaderModule(device.Get(), &createInfo, nullptr, shaderModule), "Failed to create shader module");
    }

    typedef ShaderLayout::BindingType BindingType; // Why does C++ not allow class qualified using?
    DescriptorSetLayout GraphicsPipeline::createDescriptorSetLayout(const Shader &shader)
    {        
        DescriptorSetLayout::Builder builder{device};
        const ShaderLayout &layout = shader.GetLayout();
        const ShaderLayout::DescriptorSetInfo *setInfo = shader.GetLayout().GetMaterialDescriptorSet();
        if(setInfo == nullptr)
            return builder.BuildInPlace();
        for(const ShaderLayout::BindingInfo &bindingInfo : setInfo->bindings)
        {
            Console::debugf("{}", bindingInfo.binding);
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
    
    void GraphicsPipeline::createStandardLayout()
    {
        Console::log("Creating standard pipeline layout", "GraphicsPipeline");

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

    void GraphicsPipeline::createStandardPipeline(VkPipelineCache cache)
    {
        Console::log("Creating standard pipeline", "GraphicsPipeline");
        
        assert(pipelineLayout != nullptr && "Cannot create graphics pipeline:: layout is null");

        configInfo.colorAttachmentFormats = { VK_FORMAT_B8G8R8A8_SRGB };
        configInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

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

        VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            // .pNext = &flags2, // Remove for classic descriptor sets
            .pNext = nullptr,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<uint32_t>(configInfo.colorAttachmentFormats.size()),
            .pColorAttachmentFormats = configInfo.colorAttachmentFormats.data(),
            .depthAttachmentFormat = configInfo.depthAttachmentFormat,
            .stencilAttachmentFormat = configInfo.stencilAttachmentFormat
        };

        VkGraphicsPipelineCreateInfo pipelineInfo{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &pipelineRenderingCreateInfo,
            .flags = 0,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInputInfo,
            .pInputAssemblyState = &configInfo.inputAssemblyInfo,
            .pViewportState = &configInfo.viewportInfo,
            .pRasterizationState = &configInfo.rasterizationInfo,
            .pMultisampleState = &configInfo.multisampleInfo,
            .pDepthStencilState = &configInfo.depthStencilInfo,
            .pColorBlendState = &configInfo.colorBlendInfo,
            .pDynamicState = &dynamicStateInfo,

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

    void GraphicsPipeline::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }



    PipelineConfigInfo::PipelineConfigInfo()
    {
        DefaultPreset();
    }

    PipelineConfigInfo::PipelineConfigInfo(const ShaderProperties& shaderConfig)
    {
        SetShaderConfig(shaderConfig);
    }

    void PipelineConfigInfo::SetShaderConfig(const graphics::ShaderProperties& shaderConfig)
    {
        switch(shaderConfig.shaderType)
        {
            case ShaderType::LIT:
            case ShaderType::UNLIT:
            case ShaderType::SPRITE_LIT:
            case ShaderType::SPRITE_UNLIT:
            case ShaderType::UI:
                pipelineType = PipelineType::STANDARD;
                break;
            case ShaderType::POST_PROCESSING:
                pipelineType = PipelineType::POST_PROCESSING;
                break;
            default:
                Console::warn("Unrecognized shader type, defaulting to STANDARD pipeline", "PipelineConfigInfo");
                pipelineType = PipelineType::STANDARD;
        }

        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.pNext = nullptr;
        switch(shaderConfig.primitiveTopology)
        {
            case PrimitiveTopology::POINT_LIST:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
                inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
                break;
            case PrimitiveTopology::LINE_LIST:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
                inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
                break;
            case PrimitiveTopology::LINE_STRIP:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
                inputAssemblyInfo.primitiveRestartEnable = VK_TRUE;
                break;
            case PrimitiveTopology::TRIANGLE_LIST:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
                break;
            case PrimitiveTopology::TRIANGLE_STRIP:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
                inputAssemblyInfo.primitiveRestartEnable = VK_TRUE;
                break;
            case PrimitiveTopology::TRIANGLE_FAN:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
                inputAssemblyInfo.primitiveRestartEnable = VK_TRUE;
                break;
            case PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
                inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
                break;
            case PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
                inputAssemblyInfo.primitiveRestartEnable = VK_TRUE;
                break;
            case PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
                inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
                break;
            case PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
                inputAssemblyInfo.primitiveRestartEnable = VK_TRUE;
                break;
            default:
                Console::warn("Unrecognized primitive topology, defaulting to TRIANGLE_LIST", "PipelineConfigInfo");
                inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                break;
        }
    
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.pNext = nullptr;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = nullptr;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = nullptr;

        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.pNext = nullptr;
        rasterizationInfo.depthClampEnable = VK_FALSE;
        rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        switch(shaderConfig.drawMode)
        {
            case DrawMode::FILL:
                rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
                break;
            case DrawMode::WIREFRAME:
                rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
                break;
            case DrawMode::POINTS:
                rasterizationInfo.polygonMode = VK_POLYGON_MODE_POINT;
                break;
            default:
                Console::warn("Unrecognized draw mode, defaulting to FILL", "PipelineConfigInfo");
                rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
                break;
        }
        rasterizationInfo.lineWidth = 1.0f;
        switch(shaderConfig.cullMode)
        {
            case CullMode::NONE:
                rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
                break;
            case CullMode::FRONT:
                rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
                break;
            case CullMode::BACK:
                rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
                break;
            case CullMode::BOTH:
                rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
                break;
            default:
                Console::warn("Unrecognized cull mode, defaulting to BACK", "PipelineConfigInfo");
                rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
                break;
        }
        rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationInfo.depthBiasEnable = VK_FALSE;
        rasterizationInfo.depthBiasConstantFactor = 0.0f;
        rasterizationInfo.depthBiasClamp = 0.0f;
        rasterizationInfo.depthBiasSlopeFactor = 0.0f;

        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.pNext = nullptr;
        multisampleInfo.flags = 0;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleInfo.minSampleShading = 1.0f;
        multisampleInfo.pSampleMask = nullptr;
        multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleInfo.alphaToOneEnable = VK_FALSE;

        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        switch(shaderConfig.blendMode)
        {
            case BlendMode::OPAQUE:
                colorBlendAttachment.blendEnable = VK_FALSE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
                break;
            case BlendMode::ALPHA:
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
                break;
            case BlendMode::ADDITIVE:
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
                break;
            case BlendMode::MULTIPLY:
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
                break;
            case BlendMode::PREMULTIPLY:
                colorBlendAttachment.blendEnable = VK_TRUE;
                colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
                colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
                colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
                break;
        }

        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.pNext = nullptr;
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &colorBlendAttachment;
        colorBlendInfo.blendConstants[0] = 0.0f;
        colorBlendInfo.blendConstants[1] = 0.0f;
        colorBlendInfo.blendConstants[2] = 0.0f;
        colorBlendInfo.blendConstants[3] = 0.0f;

        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.pNext = nullptr;
        depthStencilInfo.depthTestEnable = VK_TRUE;
        switch(shaderConfig.depthTest)
        {
            case DepthTest::NEVER:
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_NEVER;
                break;
            case DepthTest::LESS:
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                break;
            case DepthTest::LESS_EQUAL:
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                break;
            case DepthTest::EQUAL:
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_EQUAL;
                break;
            case DepthTest::GREATER_EQUAL:
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
                break;
            case DepthTest::GREATER:
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_GREATER;
                break;
            case DepthTest::NOT_EQUAL:
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
                break;
            case DepthTest::ALWAYS:
                depthStencilInfo.depthTestEnable = VK_FALSE; // Disable depth test for ALWAYS, fragment always passes
                depthStencilInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
                break;
        }
        switch(shaderConfig.depthWrite)
        {
            case DepthWrite::AUTO:
                depthStencilInfo.depthWriteEnable = 
                    shaderConfig.blendMode == BlendMode::OPAQUE ||
                    !(shaderConfig.shaderType == ShaderType::SPRITE_LIT ||
                    shaderConfig.shaderType == ShaderType::SPRITE_UNLIT ||
                    shaderConfig.shaderType == ShaderType::UI ||
                    shaderConfig.shaderType == ShaderType::POST_PROCESSING);
                break;
            case DepthWrite::ENABLED:
                depthStencilInfo.depthWriteEnable = VK_TRUE;
                break;
            case DepthWrite::DISABLED:
                depthStencilInfo.depthWriteEnable = VK_FALSE;
                break;
        }
        depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilInfo.minDepthBounds = 0.0f;
        depthStencilInfo.maxDepthBounds = 1.0f;
        depthStencilInfo.stencilTestEnable = VK_FALSE;
        depthStencilInfo.front = {};
        depthStencilInfo.back = {};

        dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.pNext = nullptr;
        dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
        dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
        dynamicStateInfo.flags = 0;
    }

}