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
#include "descriptor_set.hpp"

namespace graphics
{
enum class PipelineType
{
    STANDARD = 0,
    POST_PROCESSING = 1,
    ID_BUFFER = 2
};

struct PipelineConfigInfo 
{
    PipelineConfigInfo();
    PipelineConfigInfo(const ShaderProperties&);
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    PipelineType pipelineType = PipelineType::STANDARD;

    VkPipelineViewportStateCreateInfo viewportInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

    std::vector<VkDynamicState> dynamicStateEnables{};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};

    std::vector<VkFormat> colorAttachmentFormats{};
    VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

    void SetShaderConfig(const graphics::ShaderProperties& shaderConfig);

    constexpr void DefaultPreset()
    {
        pipelineType = PipelineType::STANDARD;

        inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    
        viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportInfo.viewportCount = 1;
        viewportInfo.pViewports = NULL;
        viewportInfo.scissorCount = 1;
        viewportInfo.pScissors = NULL;

        rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationInfo.depthClampEnable = VK_FALSE;
        rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationInfo.lineWidth = 1.0f;
        rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationInfo.depthBiasEnable = VK_FALSE;
        rasterizationInfo.depthBiasConstantFactor = 0.0f;
        rasterizationInfo.depthBiasClamp = 0.0f;
        rasterizationInfo.depthBiasSlopeFactor = 0.0f;

        multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleInfo.sampleShadingEnable = VK_FALSE;
        multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleInfo.minSampleShading = 1.0f;
        multisampleInfo.pSampleMask = nullptr;
        multisampleInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleInfo.alphaToOneEnable = VK_FALSE;

        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendInfo.logicOpEnable = VK_FALSE;
        colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendInfo.attachmentCount = 1;
        colorBlendInfo.pAttachments = &colorBlendAttachment;
        colorBlendInfo.blendConstants[0] = 0.0f;
        colorBlendInfo.blendConstants[1] = 0.0f;
        colorBlendInfo.blendConstants[2] = 0.0f;
        colorBlendInfo.blendConstants[3] = 0.0f;

        depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilInfo.depthTestEnable = VK_TRUE;
        depthStencilInfo.depthWriteEnable = VK_TRUE;
        depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        depthStencilInfo.minDepthBounds = 0.0f;
        depthStencilInfo.maxDepthBounds = 1.0f;
        depthStencilInfo.stencilTestEnable = VK_FALSE;
        depthStencilInfo.front = {};
        depthStencilInfo.back = {};

        dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStateInfo.pDynamicStates = nullptr; // Set in constructor
        dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
        dynamicStateInfo.flags = 0;
        dynamicStateInfo.pNext = nullptr;
    }

    void TransparentPreset()
    {
        DefaultPreset();
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    }
};

class GraphicsPipeline
{
public:
    GraphicsPipeline(internal::Device &device, id_t id, const Shader &shader, VkPipelineCache cache);
    // GraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, int id, VkPipelineLayout layout);
    ~GraphicsPipeline();

    GraphicsPipeline(const GraphicsPipeline&) = delete;
    GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

    void Bind(VkCommandBuffer commandBuffer);

    id_t GetID() const { return ID; }
    VkPipelineLayout GetPipelineLayout() { return pipelineLayout; }

private:
    internal::Device &device;
    VkPipeline graphicsPipeline = nullptr;
    VkPipelineLayout pipelineLayout = nullptr;

    PipelineConfigInfo configInfo;
    std::vector<VkDynamicState> dynamicStateEnables {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR}; // TODO: Make configurable
    VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo;

    id_t ID = -1;

    std::unique_ptr<DescriptorSetLayout> materialSetLayout{};
    
    VkShaderModule vertexShaderModule = nullptr;
    VkShaderModule fragmentShaderModule = nullptr;

    void createShaderModules(const Shader &shader);
    void createShaderModule(const std::vector<uint32_t>& spvCode, VkShaderModule* shaderModule);
    void createDescriptors(const Shader &shader);


    void createStandardPipeline(VkPipelineCache cache);
    // void createPostProcessingPipeline(VkPipelineCache cache);
    // void createIDBufferPipeline(VkPipelineCache cache);
    // void createGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, VkPipelineLayout layout);
    void createStandardLayout();
    // void createPostProcessingLayout();
    // void createIDBufferLayout();
};
}