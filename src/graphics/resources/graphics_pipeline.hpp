#pragma once
#include <string>
#include <vector>
#include "graphics/backend/vulkan_include.h"

#include "graphics/backend/device.hpp"
#include "engine_types.hpp"
// #include "graphics/shader.hpp"
// #include "graphics/containers.hpp"
#include "graphics/api/resources/shader.hpp"
#include "graphics/api/shader_config.hpp"
#include "descriptors.hpp"

namespace graphics
{
enum class PipelineType
{
    Standard = 0,
    PostProcessing = 1,
    IDBuffer = 2
};

/// @brief Specifies the primitive topology class. Used to prevent invalid swaps between topologies,
/// such as from line list to triangle list.
enum class PipelineTopologyClass
{
    Point,
    Line,
    Triangle,
    Patch
};

struct PipelineConfigInfo 
{
    PipelineConfigInfo();
    PipelineConfigInfo(const ShaderProperties&);
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo(PipelineConfigInfo&&);
    PipelineConfigInfo&& operator=(PipelineConfigInfo&&) = delete;

    PipelineType pipelineType = PipelineType::Standard;
    PipelineTopologyClass topologyClass = PipelineTopologyClass::Triangle;

    vk::PipelineViewportStateCreateInfo viewportInfo{};
    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    vk::PipelineRasterizationStateCreateInfo rasterizationInfo{};
    vk::PipelineMultisampleStateCreateInfo multisampleInfo{};
    vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
    vk::PipelineColorBlendStateCreateInfo colorBlendInfo{};
    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{};

    std::vector<vk::Format> colorAttachmentFormats = { vk::Format::eB8G8R8A8Srgb };
    vk::Format depthAttachmentFormat = vk::Format::eD32Sfloat;
    vk::Format stencilAttachmentFormat = vk::Format::eUndefined;

    const std::vector<vk::DynamicState> dynamicStateEnables {
        vk::DynamicState::eViewport, vk::DynamicState::eScissor, // Controlled dynamically by viewport image
        vk::DynamicState::ePrimitiveTopology, // Determined by mesh data. TODO: Reenable after mesh rewrite
        vk::DynamicState::ePrimitiveRestartEnable,
        // vk::DynamicState::ePolygonModeEXT, // Can be overridden

        // Look into:
        // VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,
        // VK_DYNAMIC_STATE_LINE_STIPPLE
    };
    vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};

    void SetShaderConfig(const graphics::ShaderProperties& shaderConfig);

    void DefaultPreset()
    {
        pipelineType = PipelineType::Standard;

        inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
        inputAssemblyInfo.primitiveRestartEnable = false;
        topologyClass = PipelineTopologyClass::Triangle;
    
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = VK_NULL_HANDLE;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = VK_NULL_HANDLE;

        rasterizationInfo.depthClampEnable = false;
        rasterizationInfo.rasterizerDiscardEnable = false;
        rasterizationInfo.polygonMode = vk::PolygonMode::eFill;
        rasterizationInfo.lineWidth = 1.0f;
        rasterizationInfo.cullMode = vk::CullModeFlagBits::eBack;
        rasterizationInfo.frontFace = vk::FrontFace::eClockwise;
        rasterizationInfo.depthBiasEnable = false;
        rasterizationInfo.depthBiasConstantFactor = 0.0f;
        rasterizationInfo.depthBiasClamp = 0.0f;
        rasterizationInfo.depthBiasSlopeFactor = 0.0f;

        multisampleInfo.sampleShadingEnable = false;
        multisampleInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
        multisampleInfo.minSampleShading = 1.0f;
        multisampleInfo.pSampleMask = VK_NULL_HANDLE;
        multisampleInfo.alphaToCoverageEnable = false;
        multisampleInfo.alphaToOneEnable = false;

        colorBlendAttachment.colorWriteMask = 
            vk::ColorComponentFlagBits::eR | 
            vk::ColorComponentFlagBits::eG | 
            vk::ColorComponentFlagBits::eB | 
            vk::ColorComponentFlagBits::eA;
        colorBlendAttachment.blendEnable = false;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

        colorBlendInfo.logicOpEnable = false;
        colorBlendInfo.logicOp = vk::LogicOp::eCopy;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &colorBlendAttachment;
        colorBlendInfo.blendConstants[0] = 0.0f;
        colorBlendInfo.blendConstants[1] = 0.0f;
        colorBlendInfo.blendConstants[2] = 0.0f;
        colorBlendInfo.blendConstants[3] = 0.0f;

        depthStencilInfo.depthTestEnable = true;
        depthStencilInfo.depthWriteEnable = true;
        depthStencilInfo.depthCompareOp = vk::CompareOp::eGreater; // Reversed depth
        depthStencilInfo.depthBoundsTestEnable = false;
        depthStencilInfo.minDepthBounds = 0.0f;
        depthStencilInfo.maxDepthBounds = 1.0f;
        depthStencilInfo.stencilTestEnable = false;
        depthStencilInfo.front = vk::StencilOpState{};
        depthStencilInfo.back = vk::StencilOpState{};

        dynamicStateInfo.pDynamicStates = dynamicStateEnables.data();
        dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
    }

    void TransparentPreset()
    {
        DefaultPreset();
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
        colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
        colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
        colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;
    }
};

class GraphicsPipelineOld // TODO: Rewrite entirely. Maybe merge with compute pipeline.
{
public:
    GraphicsPipelineOld(internal::Device &device, id_t id, const Shader &shader, VkPipelineCache cache);
    // GraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, int id, VkPipelineLayout layout);
    ~GraphicsPipelineOld();

    GraphicsPipelineOld(const GraphicsPipelineOld&) = delete;
    GraphicsPipelineOld& operator=(const GraphicsPipelineOld&) = delete;

    void Bind(VkCommandBuffer commandBuffer);

    id_t GetID() const { return ID; }
    VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
    const DescriptorSetLayout &GetDescriptorSetLayout() const { return materialSetLayout; }

private:
    internal::Device &device;
    VkPipeline graphicsPipeline = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;

    PipelineConfigInfo configInfo;
    std::vector<VkDynamicState> dynamicStateEnables {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}; // TODO: Make configurable
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo;

    id_t ID = -1;

    DescriptorSetLayout materialSetLayout;
    
    VkShaderModule vertexShaderModule = nullptr;
    VkShaderModule fragmentShaderModule = nullptr;

    void createShaderModules(const Shader &shader);
    void createShaderModule(const std::vector<uint32_t>& spvCode, VkShaderModule* shaderModule);
    DescriptorSetLayout createDescriptorSetLayout(const Shader &shader);


    void createStandardPipeline(VkPipelineCache cache);
    // void createPostProcessingPipeline(VkPipelineCache cache);
    // void createIDBufferPipeline(VkPipelineCache cache);
    // void createGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, VkPipelineLayout layout);
    void createStandardLayout();
    // void createPostProcessingLayout();
    // void createIDBufferLayout();
};

/// @brief Abstraction of a VkPipeline used for graphics. 
/// Pipelines are immutable, so the pipeline must be destroyed and recreated to make changes.
class GraphicsPipeline
{
public:
    GraphicsPipeline(internal::Device &device, const Shader &shader, vk::PipelineCache cache);
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;
    GraphicsPipeline (GraphicsPipeline&&); // Allow explicit moving
    GraphicsPipeline&& operator=(GraphicsPipeline&&) = delete;

    void Bind(vk::CommandBuffer commandBuffer);

    VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
    const DescriptorSetLayout &GetDescriptorSetLayout() const { return m_materialSetLayout; }
private:
    internal::Device& m_device;

    vk::PipelineLayout m_pipelineLayout = nullptr;
    vk::Pipeline m_pipeline = nullptr;
    DescriptorSetLayout m_materialSetLayout;

    PipelineConfigInfo m_configInfo;

    DescriptorSetLayout createDescriptorSetLayout(const Shader &shader);

    void createPipelineLayout(const Shader &shader);
    vk::ShaderModuleCreateInfo createShaderModuleInfo(const std::vector<uint32_t> &spvCode);
    std::vector<vk::VertexInputBindingDescription> createVertexBindings(const VertexLayout& layout);
    std::vector<vk::VertexInputAttributeDescription> createVertexAttributes(const VertexLayout& layout);
    void createPipeline(const Shader &shader, vk::PipelineCache cache);

};

}