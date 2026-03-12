#include "mesh_manager.hpp"

#include "utils/console.hpp"

namespace graphics
{

MeshHandle MeshManager::RegisterMesh(const core::MeshData &meshData)
{

}

bool MeshManager::UpdateMesh(const core::MeshData &meshData)
{

}

bool MeshManager::UnregisterMesh(MeshHandle handle)
{

}

/// @brief Get a pointer to the GraphicsMesh stored at the handle
/// @param handle 
/// @return nullptr if invalid, pointer if valid
GraphicsMesh* MeshManager::GetMesh(MeshHandle handle)
{
    if(!IsValid(handle))
    {
        Console::errorf("GraphicsMesh at index {} is invalid.", handle.index, "MeshManager");
        return nullptr;
    }
}

/// @brief Check the validity of a mesh handle
/// @param handle
/// @return True if valid, false if invalid
bool MeshManager::IsValid(MeshHandle handle) const
{
    return handle.IsValid() && // Check if index is valid
        handle.generation < entries[handle.index].generation; // Check for UAF
}

} // namespace graphics