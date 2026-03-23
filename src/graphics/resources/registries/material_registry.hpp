#pragma once
#include "registry.hpp"
#include "graphics/resources/graphics_material.hpp"
#include "graphics/api/resources/material.hpp"

namespace graphics
{

class GraphicsPipeline;
class MaterialRegistry : public GraphicsRegistry<GraphicsMaterial, MaterialHandle>
{
public:
    MaterialHandle Register(Material &material, const GraphicsPipeline &pipeline, DescriptorPool &pool);
    bool Update(Material &material);
    using GraphicsRegistry<GraphicsMaterial, MaterialHandle>::Deregister;
    inline bool Deregister(Material &material)
    {
        bool result = Deregister(material.graphicsHandle);
        return result;
    }
};

};