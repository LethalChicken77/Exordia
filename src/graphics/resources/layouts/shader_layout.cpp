#include "shader_layout.hpp"
#include "spirv_reflect.h"
#include "utils/console.hpp"
#include "graphics/backend/vulkan_include.h"
#include <sstream>

namespace graphics
{

// enum class BindingType : uint8_t
// {
//     UniformBuffer,
//     StorageBuffer,
//     DynamicUniformBuffer,
//     DynamicStorageBuffer,
    
//     SampledImage,
//     CombinedImageSampler,
//     StorageImage,
//     Sampler,
//     UniformTexelBuffer,
//     StorageTexelBuffer,
//     InputAttachment,

//     AccelerationStructure,
//     InlineUniformBlock,

//     Invalid = 255
// };

std::string ShaderParameter::ToString() const
{
    return std::format("{{{:<18}: Offset: {:>5}, Type: {:>6}}}",
        name,
        offset,
        type.ToString());
}

std::string BufferLayout::ToString() const
{
    std::stringstream result{};
    result << "Buffer layout:\n";
    for(const ShaderParameter& param : parameters)
    {
        result << "\t" << param.ToString() << "\n";
    }
    return result.str();
}

std::string ShaderLayout::ToString() const
{
    std::stringstream result{};
    result << "Shader layout:\n";
    for(const DescriptorSetInfo& ds : descriptorSets)
    {
        result << "Descriptor set " << ds.id << " {\n";
        for(const BindingInfo& bindingInfo : ds.bindings)
        {
            result << std::format("\tBinding {} ({}):\n", bindingInfo.name, bindingInfo.binding);
            result << std::format("\t\t{{Count: {}, Type: {}}}\n", bindingInfo.count, bindingInfo.type.ToString());
            if(bindingInfo.bufferIndex != ~0u)
            {
                result << "\t" << bufferLayouts[bindingInfo.bufferIndex].ToString() << "\n";
            }
            // else if(bindingInfo)
            // {

            // }
        }
    }
    result << "}";
    return result.str();
}

} // namespace graphics