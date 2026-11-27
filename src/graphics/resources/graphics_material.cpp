#include "graphics_material.hpp"
#include "console.hpp"
#include "graphics_pipeline.hpp"

namespace graphics
{

GraphicsMaterial::GraphicsMaterial(Material _base, const GraphicsPipeline &pipeline, const TextureRegistry& textureRegistry, DescriptorPool &pool) 
    : device(pool.GetDevice()),
    descriptorPool(pool),
    layout(&pipeline.GetDescriptorSetLayout())
{
    shaderHandle = _base->shader->graphicsHandle;
    name = _base->name;
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
void GraphicsMaterial::UpdateMaterial(Material _base, const GraphicsPipeline &pipeline, const TextureRegistry& textureRegistry)
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
    if(_base->GetData().size() != 0)
    {
        if(ubo == nullptr || ubo->GetSize() != _base->GetBufferSize())
        {
            ubo = std::make_unique<Buffer>(
                device,
                _base->GetData().size(),
                1,
                vk::BufferUsageFlagBits::eUniformBuffer,
                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
                1
            );
        }
    
        VkDescriptorBufferInfo bufferInfo = ubo->GetDescriptorInfo();
        writer.WriteBuffer(MATERIAL_BUFFER_BINDING, bufferInfo);
        
        ubo->Map();
        ubo->WriteData((void*)_base->GetData().data(), _base->GetData().size());
        ubo->Unmap();
        empty = false;
    }

    for(const MaterialData::TextureBinding &binding : _base->GetTextureBindings())
    {
        TextureResource *tex = textureRegistry.Get(binding.handle);
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

void GraphicsMaterial::updateData(Material base)
{
}

} // namespace graphics