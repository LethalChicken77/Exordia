#pragma once
#include "registry.hpp"
#include "graphics/resources/graphics_mesh.hpp"

namespace graphics
{
    
class MeshRegistry : public GraphicsRegistry<GraphicsMesh, MeshHandle>
{
public:
    MeshHandle Register(core::MeshData &meshData);
    bool Update(core::MeshData &meshData);
    using GraphicsRegistry<GraphicsMesh, MeshHandle>::Deregister;
    inline bool Deregister(core::MeshData &meshData)
    {
        return Deregister(meshData.graphicsHandle);
    }
};

} // namespace graphics