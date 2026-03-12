#include "mesh_manager.hpp"

namespace graphics
{

/// @brief Create graphics mesh data for a given mesh
/// @param meshData 
/// @return Handle for new mesh, invalid on failure
MeshHandle MeshManager::RegisterMesh(core::MeshData &meshData)
{
    if(meshData.graphicsHandle.IsValid()) return meshData.graphicsHandle; // Mesh is already registered

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
    entry.mesh = std::make_unique<GraphicsMesh>(&meshData);
    entry.inUse = true;
    entry.generation = nextGeneration++;

    meshData.graphicsHandle = {index, entry.generation};
    return meshData.graphicsHandle;
}

/// @brief Update the graphics mesh corresponding to an already registered mesh
/// @param meshData 
/// @return True on success, false on failure
bool MeshManager::UpdateMesh(core::MeshData &meshData)
{
    MeshHandle &handle = meshData.graphicsHandle;
    if(!meshData.graphicsHandle.IsValid()) return false;

    Entry &entry = entries[handle.index];

    entry.mesh->createBuffers(&meshData);

    return true;
}

/// @brief Deregister a graphics mesh
/// @param handle 
/// @return True if successful, false otherwise
bool MeshManager::DeregisterMesh(MeshHandle handle)
{
    if(!IsValid(handle))
    {
        return false;
    }

    Entry &entry = entries[handle.index];
    entry.mesh.reset(); // TODO: Deferred deletion to avoid errors during a frame
    entry.generation = ~0u;
    entry.inUse = false;
    
    return true;
}

/// @brief Get a pointer to the GraphicsMesh stored at the handle
/// @param handle 
/// @return nullptr if invalid, pointer if valid
const GraphicsMesh* MeshManager::GetMesh(MeshHandle handle)
{
    if(!IsValid(handle))
    {
        Console::errorf("GraphicsMesh at index {} is invalid.", handle.index, "MeshManager");
        return nullptr;
    }
    return entries[handle.index].mesh.get();
}

/// @brief Check the validity of a mesh handle
/// @param handle
/// @return True if valid, false if invalid
bool MeshManager::IsValid(MeshHandle handle) const
{
    return handle.IsValid() && // Check if index is valid
        handle.index < entries.size() && // Check if index is in bounds
        entries[handle.index].inUse &&
        entries[handle.index].mesh != nullptr &&
        handle.generation == entries[handle.index].generation; // Check for UAF
}

} // namespace graphics