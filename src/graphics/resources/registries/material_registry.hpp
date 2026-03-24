#pragma once
#include "registry.hpp"
#include "graphics/resources/graphics_material.hpp"
#include "graphics/api/resources/material.hpp"
#include "pipeline_registry.hpp"

namespace graphics
{

class GraphicsPipeline;
class MaterialRegistry : public GraphicsRegistry<GraphicsMaterial, MaterialHandle>
{
public:
    MaterialRegistry(const PipelineRegistry &pipelineRegistry, const TextureRegistry &textureRegistry);

    MaterialHandle Register(Material &material, DescriptorPool &pool);
    bool Update(Material &material);
    using GraphicsRegistry<GraphicsMaterial, MaterialHandle>::Deregister;
    inline bool Deregister(Material &material)
    {
        return Deregister(material.graphicsHandle);
    }
private:
    const PipelineRegistry &pipelineRegistry;
    const TextureRegistry &textureRegistry;
};

};