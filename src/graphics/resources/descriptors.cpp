#include "descriptors.hpp"

#include "graphics/graphics_data.hpp"

#include "utils/console.hpp"
#include "utils/debug.hpp"

#include <ranges>

namespace graphics
{

DescriptorSetLayout::Builder::Builder() : Builder(graphicsData->GetBackend().GetDevice()) {}

/// @brief Add a binding to the new descriptor set layout
/// @param binding Binding index
/// @param descriptorType Type of the descriptor (buffer, image, etc.)
/// @param stageFlags Shader stages that will access this binding
/// @param count Number of descriptors in the binding
/// @return Reference to the builder for chaining
DescriptorSetLayout::Builder &DescriptorSetLayout::Builder::AddBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count)
{
    if(bindings.count(binding) > 0)
    {
        Console::error(std::format("Binding {} already in use in DescriptorSetLayout::Builder", binding), "[DescriptorSetLayout::Builder]");
        return *this;
    }
    VkDescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings.insert_or_assign(binding, layoutBinding);
    return *this;
}

/// @brief Build the descriptor set layout in place
/// @return Layout object
DescriptorSetLayout DescriptorSetLayout::Builder::BuildInPlace() const 
{
    return std::move(DescriptorSetLayout(device, bindings));
}

/// @brief Build the descriptor set layout
/// @return Unique pointer to the created descriptor set layout
std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const 
{
    return std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout(device, bindings));
}

DescriptorSetLayout::DescriptorSetLayout(internal::Device &device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) 
    : device(device), 
    bindings(bindings)
{
    std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};

    for(const VkDescriptorSetLayoutBinding &binding : bindings | std::views::values)
    {
        setLayoutBindings.push_back(binding);
    }

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    createInfo.pBindings = setLayoutBindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(
        device.Get(),
        &createInfo,
        nullptr,
        &descriptorSetLayout), "Failed to create descriptor set layout");
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) : device(other.device), descriptorSetLayout(other.descriptorSetLayout) 
{
    other.descriptorSetLayout = VK_NULL_HANDLE;
}

DescriptorSetLayout::~DescriptorSetLayout()
{
    if(descriptorSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device.Get(), descriptorSetLayout, nullptr);
    }
}

/// @brief Add a pool size to the descriptor pool builder
/// @param type Type of the pool size
/// @param size Size of the pool
DescriptorPool::Builder &DescriptorPool::Builder::AddPoolSize(VkDescriptorType type, uint32_t size)
{
    poolSizes.push_back({type, size});
    return *this;
}

/// @brief Set the pool flags for the descriptor pool builder
/// @param flags 
DescriptorPool::Builder &DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags)
{
    poolFlags = flags;
    return *this;
}

/// @brief Set the max number of descriptor sets the pool can allocate
/// @param newMax 
DescriptorPool::Builder &DescriptorPool::Builder::SetMaxSets(uint32_t newMax)
{
    maxSets = newMax;
    return *this;
}

/// @brief Construct the pool
/// @return Unique pointer to the created descriptor pool
std::unique_ptr<DescriptorPool> DescriptorPool::Builder::Build()
{
    return std::unique_ptr<DescriptorPool>(new DescriptorPool(device, poolSizes, poolFlags, maxSets));
}

DescriptorPool::DescriptorPool(internal::Device &_device, const std::vector<VkDescriptorPoolSize> &poolSizes, VkDescriptorPoolCreateFlags flags, uint32_t maxSets)
    : device(_device)
{
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.flags = flags;
    createInfo.maxSets = maxSets;
    createInfo.pNext = nullptr;

    VK_CHECK(vkCreateDescriptorPool(device.Get(), &createInfo, nullptr, &pool), "Failed to create descriptor pool");
}

DescriptorPool::~DescriptorPool()
{
    if(pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device.Get(), pool, nullptr);
}


bool DescriptorPool::AllocateDescriptor(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const 
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1;
    // TODO: Handle pool overallocation better, ie. creating a new pool
    VkResult result = VK_SUCCESS;
    if ((result = vkAllocateDescriptorSets(device.Get(), &allocInfo, &descriptor)) != VK_SUCCESS) 
    {
        Console::errorf("Failed to allocate descriptor sets (Error {}): {}", (uint32_t)result, Debug::VkResultToString(result), "DescriptorPool");
        return false;
    }
    return true;
}

void DescriptorPool::FreeDescriptors(std::vector<VkDescriptorSet> &descriptors) const 
{
    vkFreeDescriptorSets(
        device.Get(),
        pool,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data()
    );
}

void DescriptorPool::ResetPool() 
{
    vkResetDescriptorPool(device.Get(), pool, 0);
}

DescriptorWriter::DescriptorWriter(const DescriptorSetLayout &setLayout, const DescriptorPool &pool)
: setLayout{setLayout}, pool{pool} {}

DescriptorWriter::DescriptorWriter(const DescriptorSetLayout *setLayout, const DescriptorPool *pool)
    : setLayout{*setLayout}, pool{*pool} {}

DescriptorWriter::DescriptorWriter(const std::unique_ptr<DescriptorSetLayout> &setLayout, const std::unique_ptr<DescriptorPool> &pool)
    : setLayout{*setLayout}, pool{*pool} {}

DescriptorWriter &DescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo) 
{
    const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> &bindings = setLayout.GetBindings();
    if(bindings.count(binding) != 1)
    {
        Console::errorf("Layout does not contain binding {}", binding, "DescriptorWriter");
        return *this; // Should maybe throw
    }

    const VkDescriptorSetLayoutBinding &bindingDescription = bindings.at(binding);
    
    if(bindingDescription.descriptorCount != 1)
    {
        Console::errorf("Binding {} expects multiple descriptors", binding, "DescriptorWriter");
        return *this;
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = bufferInfo;
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
}

DescriptorWriter &DescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo *imageInfo) 
{
    const std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> &bindings = setLayout.GetBindings();
    if(bindings.count(binding) != 1)
    {
        Console::errorf("Layout does not contain binding {}", binding, "DescriptorWriter");
        return *this; // Should maybe throw
    }

    const VkDescriptorSetLayoutBinding &bindingDescription = bindings.at(binding);

    if(bindingDescription.descriptorCount != 1)
    {
        Console::errorf("Binding {} expects multiple descriptors", binding, "DescriptorWriter");
        return *this;
    }

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = bindingDescription.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = imageInfo;
    write.descriptorCount = 1;

    writes.push_back(write);
    return *this;
}

bool DescriptorWriter::Build(VkDescriptorSet &set) 
{
    bool success = pool.AllocateDescriptor(setLayout.GetDescriptorSetLayout(), set);
    if(!success) 
    {
        return false;
    }
    Overwrite(set);
    return true;
}

void DescriptorWriter::Overwrite(VkDescriptorSet &set) 
{
    for(VkWriteDescriptorSet &write : writes) 
    {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(pool.device.Get(), writes.size(), writes.data(), 0, nullptr);
}

} // namespace graphics