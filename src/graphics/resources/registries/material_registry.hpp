#pragma once
#include "registry.hpp"
#include "graphics/resources/graphics_material.hpp"
#include "graphics/api/resources/material.hpp"

namespace graphics
{

class MaterialRegistry : public GraphicsRegistry<GraphicsMaterial, MaterialHandle>
{
public:
    MaterialHandle Register(Material &material);
    bool Update(Material &material);
    using GraphicsRegistry<GraphicsMaterial, MaterialHandle>::Deregister;
    // inline bool Deregister(core::MeshData &meshData)
    // {
    //     bool result = Deregister(meshData.graphicsHandle);
    //     return result;
    // }
};

};