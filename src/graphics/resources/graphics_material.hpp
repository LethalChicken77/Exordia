#pragma once
#include "descriptors.hpp"
#include "buffer.hpp"
#include "texture_resource.hpp"
#include "registries/texture_registry.hpp"

#include "graphics/backend/device.hpp"
#include "graphics/resources/layouts/shader_layout.hpp"
#include "graphics/api/handles.hpp"
#include "graphics/api/resources/material.hpp"

namespace graphics
{

class GraphicsPipeline;
class GraphicsMaterial
{
public:
    static const uint32_t MATERIAL_BUFFER_BINDING = 0;
    std::string name;
    ShaderHandle shaderHandle;

    GraphicsMaterial(Material base, const GraphicsPipeline &pipeline, const TextureRegistry& textureRegistry, DescriptorPool &pool);
    ~GraphicsMaterial();

    void UpdateMaterial(Material base, const GraphicsPipeline &pipeline, const TextureRegistry& textureRegistry);

    [[nodiscard]] VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }
    
    bool IsEmpty() const { return empty; }

private:
    internal::Device &device;
    DescriptorPool &descriptorPool;
    const DescriptorSetLayout *layout;
    ShaderLayout::DescriptorSetInfo descriptorSetInfo{};
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> ubo = nullptr; // TODO: Replace with buffer handles to allow for large buffers + bindless
    std::vector<TextureResource*> textures{};

    void updateData(Material base);
    bool empty = true;
};

} // namespace graphics