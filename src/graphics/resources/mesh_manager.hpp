#pragma once
#include <unordered_map>

#include "graphics_mesh.hpp"
#include "graphics/api/handles.hpp"

namespace graphics
{

class MeshManager
{
public:
    MeshHandle RegisterMesh(core::MeshData &meshData);
    bool UpdateMesh(core::MeshData &meshData);
    /// @brief Deregister a graphics mesh
    /// @param meshData 
    /// @return True if successful, false otherwise
    inline bool DeregisterMesh(core::MeshData &meshData)
    {
        return DeregisterMesh(meshData.graphicsHandle);
    }
    bool DeregisterMesh(MeshHandle handle);

    const GraphicsMesh* GetMesh(MeshHandle handle);
    bool IsValid(MeshHandle handle) const;

private:
    struct Entry
    {
        std::unique_ptr<GraphicsMesh> mesh{};
        uint32_t generation = 0;
        bool inUse = false;
    };
    
    std::vector<Entry> entries{};
    std::vector<uint32_t> freeList{};

    uint32_t nextGeneration = 1;
};

} // namespace graphics