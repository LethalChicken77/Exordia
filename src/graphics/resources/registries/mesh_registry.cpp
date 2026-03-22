#include "mesh_registry.hpp"

namespace graphics
{

/// @brief Create graphics mesh data for a given mesh
/// @param meshData 
/// @return Handle for new mesh, invalid on failure
MeshHandle MeshRegistry::Register(core::MeshData &meshData)
{
    if(IsValid(meshData.graphicsHandle)) return meshData.graphicsHandle; // Mesh is already registered

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
    entry.value = std::make_unique<GraphicsMesh>(&meshData);
    entry.inUse = true;
    entry.generation = nextGeneration++;

    meshData.graphicsHandle = {index, entry.generation};
    return meshData.graphicsHandle;
}

/// @brief Update the graphics mesh corresponding to an already registered mesh
/// @param meshData 
/// @return True on success, false on failure
bool MeshRegistry::Update(core::MeshData &meshData)
{
    MeshHandle &handle = meshData.graphicsHandle;
    if(!handle.IsValid()) return false;

    Entry &entry = entries[handle.index];

    entry.value->createBuffers(&meshData);

    return true;
}

} // namespace graphics