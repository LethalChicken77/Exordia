#pragma once
#include "graphics/backend/device.hpp"
#include <memory>
#include "graphics/backend/image.hpp"
#include "graphics/backend/buffer.hpp"

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
        
        std::unique_ptr<DescriptorSetLayout> Build() const;

    private:
        internal::Device &device;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    ~DescriptorSetLayout();
    DescriptorSetLayout(const DescriptorSetLayout &) = delete;

    VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }
private:
    DescriptorSetLayout(internal::Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);

    internal::Device &device;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

    friend class DescriptorBuffer;
};


// Internal layout of a descriptor
class DescriptorLayout
{
public:
    DescriptorLayout() = default;

    void AddBindingSize(uint32_t size) { elementSizes.push_back(size); }
    void AlignSizes(uint32_t alignment)
    {
        for(uint32_t &size : elementSizes)
        {
            size = (size + alignment - 1) & ~(alignment - 1);
        }
    }

    const std::vector<uint32_t> &GetElementSizes() const { return elementSizes; }
    const uint32_t GetSize() const 
    { 
        uint32_t totalSize = 0;
        for(const uint32_t &size : elementSizes)
        {
            totalSize += size;
        }
        return totalSize; 
    }
private:
    std::vector<uint32_t> elementSizes{};
};

/// @brief A set of descriptors and their associated data
/// @details A replacement for DescriptorPool and DescriptorSet from the old system, built for descriptor buffers
struct DescriptorData
{
    
    std::vector<std::unique_ptr<Buffer>> buffers{};
    std::vector<std::unique_ptr<Image>> images{};
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