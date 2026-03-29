#include "pipeline_registry.hpp"

namespace graphics
{

PipelineRegistry::PipelineRegistry(internal::Device &_device)
    : device(_device) 
{
}

PipelineRegistry::~PipelineRegistry()
{
    Cleanup();
}

void PipelineRegistry::init()
{
    createPipelineCache();
}

void PipelineRegistry::createPipelineCache()
{
    VkPipelineCacheCreateInfo cacheCreateInfo{};
    cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    if(vkCreatePipelineCache(device.Get(), &cacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline cache!");
    }
}

/// @brief Create a pipeline for a given shader
/// @param shader 
/// @return Handle for new pipeline, invalid on failure
PipelineHandle PipelineRegistry::Register(Shader &shader)
{
    if(IsValid(shader.graphicsHandle)) return shader.graphicsHandle; // Mesh is already registered

    uint32_t index;
    if(freeList.size() > 0)
    {
        // Take oldest to reduce fragmentation
        index = freeList[0];
        freeList.erase(freeList.begin());
    }
    else
    {
        index = entries.size();
        entries.emplace_back();
    }

    Entry &entry = entries[index];
    entry.value = std::make_unique<GraphicsPipeline>(
        device,
        -1,
        shader,
        pipelineCache);
    entry.inUse = true;
    entry.generation = nextGeneration++;

    shader.graphicsHandle = {index, entry.generation};
    return shader.graphicsHandle;
}

/// @brief Update the graphics mesh corresponding to an already registered mesh
/// @param meshData 
/// @return True on success, false on failure
bool PipelineRegistry::Reload(Shader &shader)
{
    PipelineHandle &handle = shader.graphicsHandle;
    if(!handle.IsValid()) return false;

    Entry &entry = entries[handle.index];

    entry.value = std::make_unique<GraphicsPipeline>(
        device,
        -1,
        shader,
        pipelineCache);

    return true;
}

/// @brief Destroy all pipelines and cleanup GPU resources
void PipelineRegistry::Cleanup()
{
    Reset();
    if(pipelineCache != VK_NULL_HANDLE)
        vkDestroyPipelineCache(device.Get(), pipelineCache, nullptr);
    pipelineCache = VK_NULL_HANDLE;
}

} // namespace graphics