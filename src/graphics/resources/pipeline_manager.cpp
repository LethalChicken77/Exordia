#include "pipeline_manager.hpp"
#include "utils/console.hpp"

namespace graphics{

PipelineManager::PipelineManager(internal::Device& _device) : device(_device) {}

PipelineManager::~PipelineManager()
{
    if(pipelineCache != nullptr)
        vkDestroyPipelineCache(device.Get(), pipelineCache, nullptr);
}

void PipelineManager::init()
{
    createPipelineCache();
    CreatePipelines();
}

void PipelineManager::ReloadPipelines()
{
    device.WaitIdle();
    DestroyPipelines();
    CreatePipelines();
}

void PipelineManager::CreatePipelines()
{
    // assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
    Console::log("Creating pipelines", "PipelineManager");
    currentID = 0;
    // PipelineConfigInfo pipelineConfig{};
    // GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);

    int numShaders = shaders.size();
    for(core::Shader *shader : shaders)
    {    
        std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<GraphicsPipeline>(
            device,
            currentID++,
            *shader,
            pipelineCache
        );

        graphicsPipelines.push_back(std::move(graphicsPipeline));
    }
    Console::log("Pipelines created successfully", "PipelineManager");
}

void PipelineManager::DestroyPipelines()
{
    // for(GraphicsPipeline &graphicsPipeline : graphicsPipelines)
    // {
    //     graphicsPipeline.();
    // }
    graphicsPipelines.clear();
}

void PipelineManager::createPipelineCache()
{
    VkPipelineCacheCreateInfo cacheCreateInfo{};
    cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    if(vkCreatePipelineCache(device.Get(), &cacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline cache!");
    }
}
} // namespace graphics