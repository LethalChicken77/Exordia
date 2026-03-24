#pragma once
#include "graphics/backend/device.hpp"
#include <memory>
#include "image.hpp"
#include "buffer.hpp"

namespace graphics
{

/// @brief Wrapper for a Vulkan descriptor set layouts. Includes a map of all bindings.
class DescriptorSetLayout
{
public:
    /// @brief Builder class for DescriptorSetLayout
    class Builder
    {
    public:
        Builder();
        Builder(internal::Device &device) : device(device) {};

        Builder &AddBinding(
            uint32_t binding,
            VkDescriptorType descriptorType,
            VkShaderStageFlags stageFlags,
            uint32_t count = 1);
        
        DescriptorSetLayout BuildInPlace() const;
        std::unique_ptr<DescriptorSetLayout> Build() const;

    private:
        internal::Device &device;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    ~DescriptorSetLayout();
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
    // Allow move
    DescriptorSetLayout(DescriptorSetLayout&&);
    DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

    VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }
    const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> &GetBindings() const { return bindings; }
private:
    DescriptorSetLayout(internal::Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);

    internal::Device &device;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    friend class DescriptorWriter;
};

class DescriptorPool
{
public:
    class Builder
    {
        public:
            Builder(internal::Device &device) : device(device) {}

            Builder &AddPoolSize(VkDescriptorType type, uint32_t size);
            Builder &SetPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &SetMaxSets(uint32_t newMax);

            std::unique_ptr<DescriptorPool> Build();
        private:
            internal::Device &device;
            uint32_t maxSets = 1000;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            VkDescriptorPoolCreateFlags poolFlags = 0;
    };

    ~DescriptorPool();
    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool &operator=(const DescriptorPool &) = delete;

    bool AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

    void FreeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

    void ResetPool();

    
    const VkDescriptorPool GetPool() const { return pool; }
    internal::Device &GetDevice() { return device; } // Useful for ensuring descriptor sets track the same device as the pool
private:
    DescriptorPool(internal::Device &device, const std::vector<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags, uint32_t maxSets);
    internal::Device& device;
    VkDescriptorPool pool;

    friend class Builder;
    friend class DescriptorWriter;
};

class DescriptorWriter 
{
public:
    DescriptorWriter(const DescriptorSetLayout &setLayout, const DescriptorPool &pool);
    DescriptorWriter(const DescriptorSetLayout *setLayout, const DescriptorPool *pool);
    DescriptorWriter(const std::unique_ptr<DescriptorSetLayout> &setLayout, const std::unique_ptr<DescriptorPool> &pool);

    DescriptorWriter &WriteBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
    DescriptorWriter &WriteImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

    bool Build(VkDescriptorSet &set);
    void Overwrite(VkDescriptorSet &set);

private:
    const DescriptorSetLayout &setLayout;
    const DescriptorPool &pool;
    std::vector<VkWriteDescriptorSet> writes;
};
// TODO: Bring back descriptor writer
} // namespace graphics