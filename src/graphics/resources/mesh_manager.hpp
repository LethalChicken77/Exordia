#pragma once
#include <unordered_map>

#include "graphics_mesh.hpp"
#include "graphics/api/handles.hpp"

namespace graphics
{

class MeshManager
{
public:
    MeshHandle RegisterMesh(const core::MeshData &meshData);
    bool UpdateMesh(const core::MeshData &meshData);
    bool UnregisterMesh(MeshHandle handle);

    GraphicsMesh* GetMesh(MeshHandle handle);
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