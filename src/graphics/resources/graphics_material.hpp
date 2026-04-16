#pragma once
#include "descriptors.hpp"
#include "buffer.hpp"
#include "texture.hpp"
#include "registries/texture_registry.hpp"

#include "graphics/backend/device.hpp"
#include "graphics/api/shader_layout.hpp"
#include "graphics/api/handles.hpp"
#include "graphics/api/resources/material.hpp"

namespace graphics
{

class GraphicsPipeline;
class GraphicsMaterial
{
public:
    static const uint32_t MATERIAL_BUFFER_BINDING = 0;
    const Material* base;
    std::string name;
    GraphicsMaterial(const Material *base, const GraphicsPipeline &pipeline, const TextureRegistry& textureRegistry, DescriptorPool &pool);

    void UpdateMaterial(const Material *base);

    [[nodiscard]] VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }
    
private:
    internal::Device &device;
    const DescriptorSetLayout *layout;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> ubo = nullptr; // TODO: Replace with buffer handles to allow for large buffers + bindless
    std::vector<Texture*> textures{};
};

} // namespace graphics