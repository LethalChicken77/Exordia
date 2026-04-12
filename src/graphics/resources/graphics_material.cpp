#include "graphics_material.hpp"
#include "console.hpp"
#include "graphics_pipeline.hpp"

namespace graphics
{

GraphicsMaterial::GraphicsMaterial(const Material* _base, const GraphicsPipelineOld &pipeline, const TextureRegistry& textureRegistry, DescriptorPool &pool) 
    : device(pool.GetDevice()),
    base(_base),
    layout(&pipeline.GetDescriptorSetLayout()),
    name(_base->name)
{
    DescriptorWriter writer = DescriptorWriter(pipeline.GetDescriptorSetLayout(), pool);

    if(base->GetData().size() != 0)
    {
        ubo = std::make_unique<Buffer>(
            device,
            base->GetData().size(),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            1
        );
    
        VkDescriptorBufferInfo bufferInfo = ubo->GetDescriptorInfo();
    
        writer.WriteBuffer(MATERIAL_BUFFER_BINDING, bufferInfo);
        
        ubo->Map();
        ubo->WriteData((void*)base->GetData().data(), base->GetData().size());
        ubo->Unmap();
    }

    for(const Material::TextureBinding &binding : base->GetTextureBindings())
    {
        Texture *tex = textureRegistry.Get(binding.handle);
        if(tex == nullptr)
        {
            Console::warnf("Invalid texture handle for binding {}", binding.binding, "GraphicsMaterial");
            continue;
        }

        VkDescriptorImageInfo imageInfo = tex->GetDescriptorInfo();
        writer.WriteImage(binding.binding, imageInfo);
    }

    writer.Build(descriptorSet);
}

} // namespace graphics