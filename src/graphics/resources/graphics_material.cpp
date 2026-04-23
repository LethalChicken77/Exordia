#include "graphics_material.hpp"
#include "console.hpp"
#include "graphics_pipeline.hpp"

namespace graphics
{

GraphicsMaterial::GraphicsMaterial(const Material* _base, const GraphicsPipeline &pipeline, const TextureRegistry& textureRegistry, DescriptorPool &pool) 
    : device(pool.GetDevice()),
    descriptorPool(pool),
    base(_base),
    layout(&pipeline.GetDescriptorSetLayout()),
    name(_base->name)
{
    UpdateMaterial(_base, pipeline, textureRegistry);
}

GraphicsMaterial::~GraphicsMaterial()
{
    if(descriptorSet != VK_NULL_HANDLE)
    {
        vkFreeDescriptorSets(device.Get(), descriptorPool.GetPool(), 1, &descriptorSet);
    }
}

/// @brief Update the values of the material buffer
/// @param base Material to update the values with
/// @return 
void GraphicsMaterial::UpdateMaterial(const Material *_base, const GraphicsPipeline &pipeline, const TextureRegistry& textureRegistry)
{
    // if(descriptorSetInfo == *base->shader->GetLayout().GetMaterialDescriptorSet() && ubo->GetSize() == base->GetBufferSize())
    // {
    //     updateData(_base);
    //     return;
    // }
    // if(descriptorSet != VK_NULL_HANDLE)
    // {
    //     vkFreeDescriptorSets(device.Get(), descriptorPool.GetPool(), 1, &descriptorSet);
    //     descriptorSet = VK_NULL_HANDLE;
    // }
    DescriptorWriter writer = DescriptorWriter(pipeline.GetDescriptorSetLayout(), descriptorPool);

    empty = true;
    if(base->GetData().size() != 0)
    {
        if(ubo == nullptr || ubo->GetSize() != base->GetBufferSize())
        {
            ubo = std::make_unique<Buffer>(
                device,
                base->GetData().size(),
                1,
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                1
            );
        }
    
        VkDescriptorBufferInfo bufferInfo = ubo->GetDescriptorInfo();
        writer.WriteBuffer(MATERIAL_BUFFER_BINDING, bufferInfo);
        
        ubo->Map();
        ubo->WriteData((void*)base->GetData().data(), base->GetData().size());
        ubo->Unmap();
        empty = false;
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
        empty = false;
    }

    if(descriptorSet == VK_NULL_HANDLE)
        writer.Build(descriptorSet);
    else
        writer.Overwrite(descriptorSet);
}

void GraphicsMaterial::updateData(const Material* base)
{
}

} // namespace graphics