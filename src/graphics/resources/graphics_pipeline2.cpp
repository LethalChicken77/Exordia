#include "graphics_pipeline.hpp"
#include "graphics/graphics_data.hpp"

namespace graphics
{


GraphicsPipeline::GraphicsPipeline(internal::Device& device, const Shader &shader, VkPipelineCache cache)
    : m_device(device),
    m_materialSetLayout(createDescriptorSetLayout(shader))
{
    createPipelineLayout(shader);
    // createPipeline(cache);
}

GraphicsPipeline::~GraphicsPipeline()
{
    if(m_pipeline != nullptr)
        vkDestroyPipeline(m_device.Get(), m_pipeline, nullptr);
    if(m_pipelineLayout != nullptr)
        vkDestroyPipelineLayout(m_device.Get(), m_pipelineLayout, nullptr);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& old)
    : m_device(old.m_device),
    m_pipelineLayout(old.m_pipelineLayout),
    m_pipeline(old.m_pipeline),
    m_materialSetLayout(std::move(old.m_materialSetLayout)),
    m_configInfo(std::move(old.m_configInfo))
{
    old.m_pipelineLayout = VK_NULL_HANDLE;
    old.m_pipeline = VK_NULL_HANDLE;
}

void GraphicsPipeline::createPipelineLayout(const Shader &shader)
{
    // VkPushConstantRange pushConstantRange{};
    // pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    // pushConstantRange.offset = 0;
    // pushConstantRange.size = sizeof(PushConstants);

    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts{
        graphicsData->cameraSetLayout->GetDescriptorSetLayout(),
        graphicsData->globalSetLayout->GetDescriptorSetLayout(),
        m_materialSetLayout.GetDescriptorSetLayout()
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0; // TODO: Readd push constants
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if(m_device.Get().createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_pipelineLayout) != vk::Result::eSuccess)
    {
        throw std::runtime_error("Failed to create standard pipeline layout");
    }
}

using BindingType = ShaderLayout::BindingType;
DescriptorSetLayout GraphicsPipeline::createDescriptorSetLayout(const Shader &shader)
{        
    DescriptorSetLayout::Builder builder{m_device};
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

// ------------------------------------------------------------------------------------------------

PipelineConfigInfo::PipelineConfigInfo()
{
    DefaultPreset();
}

PipelineConfigInfo::PipelineConfigInfo(PipelineConfigInfo&& old)
    : pipelineType(old.pipelineType),
    topologyClass(old.topologyClass),
    viewportInfo(old.viewportInfo),
    inputAssemblyInfo(old.inputAssemblyInfo),
    rasterizationInfo(old.rasterizationInfo),
    multisampleInfo(old.multisampleInfo),
    colorBlendAttachment(old.colorBlendAttachment),
    colorBlendInfo(old.colorBlendInfo),
    depthStencilInfo(old.depthStencilInfo),
    colorAttachmentFormats(old.colorAttachmentFormats),
    depthAttachmentFormat(old.depthAttachmentFormat),
    stencilAttachmentFormat(old.stencilAttachmentFormat),
    dynamicStateEnables(old.dynamicStateEnables),
    dynamicStateInfo(old.dynamicStateInfo)
{
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
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
            pipelineType = PipelineType::Standard;
            break;
        case ShaderType::POST_PROCESSING:
            pipelineType = PipelineType::PostProcessing;
            break;
        default:
            Console::warn("Unrecognized shader type, defaulting to STANDARD pipeline", "PipelineConfigInfo");
            pipelineType = PipelineType::Standard;
    }

    inputAssemblyInfo.pNext = nullptr;
    switch(shaderConfig.primitiveTopology)
    {
        case PrimitiveTopology::POINT_LIST:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::ePointList;
            inputAssemblyInfo.primitiveRestartEnable = false;
            topologyClass = PipelineTopologyClass::Point;
            break;
        case PrimitiveTopology::LINE_LIST:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eLineList;
            inputAssemblyInfo.primitiveRestartEnable = false;
            topologyClass = PipelineTopologyClass::Line;
            break;
        case PrimitiveTopology::LINE_STRIP:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eLineStrip;
            inputAssemblyInfo.primitiveRestartEnable = true;
            topologyClass = PipelineTopologyClass::Line;
            break;
        case PrimitiveTopology::TRIANGLE_LIST:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
            inputAssemblyInfo.primitiveRestartEnable = false;
            topologyClass = PipelineTopologyClass::Triangle;
            break;
        case PrimitiveTopology::TRIANGLE_STRIP:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleStrip;
            inputAssemblyInfo.primitiveRestartEnable = true;
            topologyClass = PipelineTopologyClass::Triangle;
            break;
        case PrimitiveTopology::TRIANGLE_FAN:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleFan;
            inputAssemblyInfo.primitiveRestartEnable = true;
            topologyClass = PipelineTopologyClass::Triangle;
            break;
        case PrimitiveTopology::LINE_LIST_WITH_ADJACENCY:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eLineListWithAdjacency;
            inputAssemblyInfo.primitiveRestartEnable = false;
            topologyClass = PipelineTopologyClass::Line;
            break;
        case PrimitiveTopology::LINE_STRIP_WITH_ADJACENCY:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eLineStripWithAdjacency;
            inputAssemblyInfo.primitiveRestartEnable = true;
            topologyClass = PipelineTopologyClass::Line;
            break;
        case PrimitiveTopology::TRIANGLE_LIST_WITH_ADJACENCY:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleListWithAdjacency;
            inputAssemblyInfo.primitiveRestartEnable = false;
            topologyClass = PipelineTopologyClass::Triangle;
            break;
        case PrimitiveTopology::TRIANGLE_STRIP_WITH_ADJACENCY:
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleStripWithAdjacency;
            inputAssemblyInfo.primitiveRestartEnable = true;
            topologyClass = PipelineTopologyClass::Triangle;
            break;
        default:
            Console::warn("Unrecognized primitive topology, defaulting to TRIANGLE_LIST", "PipelineConfigInfo");
            inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
            inputAssemblyInfo.primitiveRestartEnable = false;
            topologyClass = PipelineTopologyClass::Triangle;
            break;
    }

    viewportInfo.pNext = nullptr;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = nullptr;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = nullptr;

    rasterizationInfo.pNext = nullptr;
    rasterizationInfo.depthClampEnable = false;
    rasterizationInfo.rasterizerDiscardEnable = false;
    switch(shaderConfig.drawMode)
    {
        case DrawMode::FILL:
            rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
            break;
        case DrawMode::WIREFRAME:
            rasterizationInfo.polygonMode = vk::PolygonMode::eLine;
            break;
        case DrawMode::POINTS:
            rasterizationInfo.polygonMode = vk::PolygonMode::ePoint;
            break;
        default:
            Console::warn("Unrecognized draw mode, defaulting to FILL", "PipelineConfigInfo");
            rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
            break;
    }
    rasterizationInfo.lineWidth = 1.0f;
    switch(shaderConfig.cullMode)
    {
        case CullMode::NONE:
            rasterizationInfo.cullMode = vk::CullModeFlagBits::eNone;
            break;
        case CullMode::FRONT:
            rasterizationInfo.cullMode = vk::CullModeFlagBits::eFront;
            break;
        case CullMode::BACK:
            rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
            break;
        case CullMode::BOTH:
            rasterizationInfo.cullMode = vk::CullModeFlagBits::eFrontAndBack;
            break;
        default:
            Console::warn("Unrecognized cull mode, defaulting to BACK", "PipelineConfigInfo");
            rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
            break;
    }
    rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
    rasterizationInfo.depthBiasEnable = false;
    rasterizationInfo.depthBiasConstantFactor = 0.0f;
    rasterizationInfo.depthBiasClamp = 0.0f;
    rasterizationInfo.depthBiasSlopeFactor = 0.0f;

    multisampleInfo.pNext = nullptr;
    multisampleInfo.flags = vk::PipelineMultisampleStateCreateFlags{};
    multisampleInfo.sampleShadingEnable = false;
    multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
    multisampleInfo.minSampleShading = 1.0f;
    multisampleInfo.pSampleMask = nullptr;
    multisampleInfo.alphaToCoverageEnable = false;
    multisampleInfo.alphaToOneEnable = false;

    colorBlendAttachment.colorWriteMask = 
        vk::ColorComponentFlagBits::eR | 
        vk::ColorComponentFlagBits::eG | 
        vk::ColorComponentFlagBits::eB | 
        vk::ColorComponentFlagBits::eA;
    switch(shaderConfig.blendMode)
    {
        case BlendMode::OPAQUE:
            colorBlendAttachment.blendEnable = false;
            colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
            colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
            colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
            break;
        case BlendMode::ALPHA:
            colorBlendAttachment.blendEnable = true;
            colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
            colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
            colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
            break;
        case BlendMode::ADDITIVE:
            colorBlendAttachment.blendEnable = true;
            colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
            colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
            break;
        case BlendMode::MULTIPLY:
            colorBlendAttachment.blendEnable = true;
            colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eDstColor;
            colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
            colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
            colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
            break;
        case BlendMode::PREMULTIPLY:
            colorBlendAttachment.blendEnable = true;
            colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
            colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
            colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
            colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
            break;
    }

    colorBlendInfo.pNext = nullptr;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = vk::LogicOp::eCopy;
    colorBlendInfo.attachmentCount = 1;
    colorBlendInfo.pAttachments = &colorBlendAttachment;
    colorBlendInfo.blendConstants[0] = 0.0f;
    colorBlendInfo.blendConstants[1] = 0.0f;
    colorBlendInfo.blendConstants[2] = 0.0f;
    colorBlendInfo.blendConstants[3] = 0.0f;

    depthStencilInfo.pNext = nullptr;
    depthStencilInfo.depthTestEnable = true;
    switch(shaderConfig.depthTest)
    {
        case CompareOp::Never:
            depthStencilInfo.depthCompareOp = vk::CompareOp::eNever;
            break;
        case CompareOp::Less:
            depthStencilInfo.depthCompareOp = graphicsData->REVERSED_DEPTH ? vk::CompareOp::eGreater : vk::CompareOp::eLess;
            break;
        case CompareOp::LessEqual:
            depthStencilInfo.depthCompareOp = graphicsData->REVERSED_DEPTH ? vk::CompareOp::eGreaterOrEqual : vk::CompareOp::eLessOrEqual;
            break;
        case CompareOp::Equal:
            depthStencilInfo.depthCompareOp = vk::CompareOp::eEqual;
            break;
        case CompareOp::GreaterEqual:
            depthStencilInfo.depthCompareOp = graphicsData->REVERSED_DEPTH ? vk::CompareOp::eLessOrEqual : vk::CompareOp::eGreaterOrEqual;
            break;
        case CompareOp::Greater:
            depthStencilInfo.depthCompareOp = graphicsData->REVERSED_DEPTH ? vk::CompareOp::eLess : vk::CompareOp::eGreater;
            break;
        case CompareOp::NotEqual:
            depthStencilInfo.depthCompareOp = vk::CompareOp::eNotEqual;
            break;
        case CompareOp::Always:
            depthStencilInfo.depthTestEnable = false; // Disable depth test for ALWAYS, fragment always passes
            depthStencilInfo.depthCompareOp = vk::CompareOp::eAlways;
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
    depthStencilInfo.front = vk::StencilOpState{};
    depthStencilInfo.back = vk::StencilOpState{};

    // dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
    dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
}

} // namespace graphics