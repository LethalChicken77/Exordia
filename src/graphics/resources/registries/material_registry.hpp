#pragma once
#include "registry.hpp"
#include "graphics/resources/graphics_material.hpp"
#include "graphics/api/resources/material.hpp"
#include "pipeline_registry.hpp"
#include "texture_registry.hpp"

namespace graphics
{

class GraphicsPipelineOld;
class MaterialRegistry : public GraphicsRegistry<GraphicsMaterial, MaterialHandle>
{
public:
    MaterialRegistry(const PipelineRegistry &pipelineRegistry, const TextureRegistry &textureRegistry);

    MaterialHandle Register(Ref<MaterialData> material, DescriptorPool &pool);
    bool Update(Ref<MaterialData> material);
    using GraphicsRegistry<GraphicsMaterial, MaterialHandle>::Deregister;
    inline bool Deregister(Ref<MaterialData> material)
    {
        return Deregister(material->graphicsHandle);
    }
private:
    const PipelineRegistry &pipelineRegistry;
    const TextureRegistry &textureRegistry;
};

};