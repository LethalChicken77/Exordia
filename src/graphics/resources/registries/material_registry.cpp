#include "material_registry.hpp"
#include "graphics/resources/graphics_pipeline.hpp"

namespace graphics
{

MaterialRegistry::MaterialRegistry(const PipelineRegistry &_pipelineRegistry)
    : pipelineRegistry(_pipelineRegistry)
{}

/// @brief Create graphics mesh data for a given mesh
/// @param meshData 
/// @return Handle for new mesh, invalid on failure
MaterialHandle MaterialRegistry::Register(Material &material, DescriptorPool &pool)
{
    if(material.graphicsHandle.IsValid()) return material.graphicsHandle; // Material is already registered

    GraphicsPipeline* pipeline = pipelineRegistry.Get(material.shaderHandle);
    if(pipeline == nullptr)
    {
        Console::error("Failed to create graphics material: Invalid pipeline", "GraphicsMaterial");
        return {};
    }

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
    entry.value = std::make_unique<GraphicsMaterial>(
        material,
        *pipeline,
        pool
    );
    entry.inUse = true;
    entry.generation = nextGeneration++;

    material.graphicsHandle = {index, entry.generation};
    return material.graphicsHandle;
}

/// @brief Update the graphics mesh corresponding to an already registered mesh
/// @param meshData 
/// @return True on success, false on failure
bool MaterialRegistry::Update(Material &material)
{
    MaterialHandle &handle = material.graphicsHandle;
    if(!material.graphicsHandle.IsValid()) return false;

    Entry &entry = entries[handle.index];

    // Update Material TODO: Implement
    // entry.value

    return true;
}

} // namespace graphics