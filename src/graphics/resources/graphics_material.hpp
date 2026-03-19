#pragma once
#include "graphics/backend/device.hpp"
#include "graphics/backend/buffer.hpp"
#include "descriptors.hpp"
#include "graphics/api/shader_layout.hpp"

namespace graphics
{

class GraphicsMaterial
{
public:
    GraphicsMaterial(const DescriptorSetLayout &layout, DescriptorPool &pool, uint32_t binding, const std::vector<uint8_t> data);

    [[nodiscard]] VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }
    
private:
    internal::Device &device;
    const DescriptorSetLayout &layout;
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    std::unique_ptr<Buffer> ubo = nullptr; // TODO: Replace with buffer handles to allow for large buffers + bindless
};

} // namespace graphics