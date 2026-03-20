#include "graphics_material.hpp"
#include "console.hpp"

namespace graphics
{

GraphicsMaterial::GraphicsMaterial(const DescriptorSetLayout &_layout, DescriptorPool &pool, uint32_t binding, const std::vector<uint8_t> data) 
    : device(pool.GetDevice()), layout(_layout)
{
    Console::log("Creating GraphicsMaterial", "GraphicsMaterial");
    DescriptorWriter writer = DescriptorWriter(layout, pool);
    if(binding != ~0u)
    {
        ubo = std::make_unique<Buffer>(
            device,
            data.size(),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            1
        );

        VkDescriptorBufferInfo bufferInfo = ubo->GetDescriptorInfo();

        writer.WriteBuffer(binding, &bufferInfo);
    }
    writer.Build(descriptorSet);
    
    ubo->Map();
    ubo->WriteData((void*)data.data(), data.size());
    ubo->Unmap();
}

} // namespace graphics