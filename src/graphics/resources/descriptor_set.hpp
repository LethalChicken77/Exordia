#pragma once
#include "graphics/backend/device.hpp"
#include <memory>

namespace graphics
{
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
private:
    DescriptorPool(internal::Device &device, const std::vector<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags, uint32_t maxSets);
    internal::Device& device;
    VkDescriptorPool pool;

    friend class Builder;
};

class DescriptorSet 
{
    public:
        class Builder 
        {
            public:
                Builder(internal::Device &device, VkDescriptorSetLayout layout) : device(device), layout(layout) {}

                DescriptorSet Build() 
                {
                    return DescriptorSet(device, layout);
                }

            private:
                internal::Device &device;
                VkDescriptorSetLayout layout;
        };
    private:
        DescriptorSet(internal::Device &device, VkDescriptorSetLayout layout) : device(device) 
        {
        }

        internal::Device &device;
        
        VkDescriptorSetLayout layout;
        VkDescriptorPool pool;

        friend class Builder;
};
} // namespace graphics