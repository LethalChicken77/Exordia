#include "var_layout.hpp"
#include "console.hpp"
#include "graphics/backend/vulkan_include.h"
#include <string>
#include <sstream>
#include <unordered_map>


namespace graphics
{

std::string TypeDescription::ToString() const
{
    return std::format("{{DataType: {}, \tCSize: {}, \tCCount: {}, \tRCount: {} \tACount: {}}}", type.ToString(), componentSize, componentCount, rowCount, arrayCount);
}

using DT = DataType;
uint32_t TypeDescription::GetVkFormat() const
{
    // AttrType type = AttrType::Float; // Datatype of the attribute. Used for format selection.
    // uint8_t componentSize = 4;
    // uint8_t componentCount = 4;
    // uint8_t arrayCount = 1; // Used for matrix types.
    static const std::unordered_map<TypeDescription, VkFormat, TypeDescription::Hash> vertexFormatTable
    {
        {{DT::UInt,  1, 1, 1}, VK_FORMAT_R8_UINT},
        {{DT::SInt,  1, 1, 1}, VK_FORMAT_R8_SINT},
        {{DT::UNorm, 1, 1, 1}, VK_FORMAT_R8_UNORM},
        {{DT::SNorm, 1, 1, 1}, VK_FORMAT_R8_SNORM},
        {{DT::UInt,  1, 2, 1}, VK_FORMAT_R8G8_UINT},
        {{DT::SInt,  1, 2, 1}, VK_FORMAT_R8G8_SINT},
        {{DT::UNorm, 1, 2, 1}, VK_FORMAT_R8G8_UNORM},
        {{DT::SNorm, 1, 2, 1}, VK_FORMAT_R8G8_SNORM},
        {{DT::UInt,  1, 3, 1}, VK_FORMAT_R8G8B8_UINT},
        {{DT::SInt,  1, 3, 1}, VK_FORMAT_R8G8B8_SINT},
        {{DT::UNorm, 1, 3, 1}, VK_FORMAT_R8G8B8_UNORM},
        {{DT::SNorm, 1, 3, 1}, VK_FORMAT_R8G8B8_SNORM},
        {{DT::UInt,  1, 4, 1}, VK_FORMAT_R8G8B8A8_UINT},
        {{DT::SInt,  1, 4, 1}, VK_FORMAT_R8G8B8A8_SINT},
        {{DT::UNorm, 1, 4, 1}, VK_FORMAT_R8G8B8A8_UNORM},
        {{DT::SNorm, 1, 4, 1}, VK_FORMAT_R8G8B8A8_SNORM},

        {{DT::UInt,  2, 1, 1}, VK_FORMAT_R16_UINT},
        {{DT::SInt,  2, 1, 1}, VK_FORMAT_R16_SINT},
        {{DT::UNorm, 2, 1, 1}, VK_FORMAT_R16_UNORM},
        {{DT::SNorm, 2, 1, 1}, VK_FORMAT_R16_SNORM},
        {{DT::Float, 2, 1, 1}, VK_FORMAT_R16_SFLOAT},
        {{DT::UInt,  2, 2, 1}, VK_FORMAT_R16G16_UINT},
        {{DT::SInt,  2, 2, 1}, VK_FORMAT_R16G16_SINT},
        {{DT::UNorm, 2, 2, 1}, VK_FORMAT_R16G16_UNORM},
        {{DT::SNorm, 2, 2, 1}, VK_FORMAT_R16G16_SNORM},
        {{DT::Float, 2, 2, 1}, VK_FORMAT_R16G16_SFLOAT},
        {{DT::UInt,  2, 3, 1}, VK_FORMAT_R16G16B16_UINT},
        {{DT::SInt,  2, 3, 1}, VK_FORMAT_R16G16B16_SINT},
        {{DT::UNorm, 2, 3, 1}, VK_FORMAT_R16G16B16_UNORM},
        {{DT::SNorm, 2, 3, 1}, VK_FORMAT_R16G16B16_SNORM},
        {{DT::Float, 2, 3, 1}, VK_FORMAT_R16G16B16_SFLOAT},
        {{DT::UInt,  2, 4, 1}, VK_FORMAT_R16G16B16A16_UINT},
        {{DT::SInt,  2, 4, 1}, VK_FORMAT_R16G16B16A16_SINT},
        {{DT::UNorm, 2, 4, 1}, VK_FORMAT_R16G16B16A16_UNORM},
        {{DT::SNorm, 2, 4, 1}, VK_FORMAT_R16G16B16A16_SNORM},
        {{DT::Float, 2, 4, 1}, VK_FORMAT_R16G16B16A16_SFLOAT},

        {{DT::UInt,  4, 1, 1}, VK_FORMAT_R32_UINT},
        {{DT::SInt,  4, 1, 1}, VK_FORMAT_R32_SINT},
        {{DT::Float, 4, 1, 1}, VK_FORMAT_R32_SFLOAT},
        {{DT::UInt,  4, 2, 1}, VK_FORMAT_R32G32_UINT},
        {{DT::SInt,  4, 2, 1}, VK_FORMAT_R32G32_SINT},
        {{DT::Float, 4, 2, 1}, VK_FORMAT_R32G32_SFLOAT},
        {{DT::UInt,  4, 3, 1}, VK_FORMAT_R32G32B32_UINT},
        {{DT::SInt,  4, 3, 1}, VK_FORMAT_R32G32B32_SINT},
        {{DT::Float, 4, 3, 1}, VK_FORMAT_R32G32B32_SFLOAT},
        {{DT::UInt,  4, 4, 1}, VK_FORMAT_R32G32B32A32_UINT},
        {{DT::SInt,  4, 4, 1}, VK_FORMAT_R32G32B32A32_SINT},
        {{DT::Float, 4, 4, 1}, VK_FORMAT_R32G32B32A32_SFLOAT},



        {{DT::Float, 2, 1}, VK_FORMAT_R16_SFLOAT},
    };

    auto it = vertexFormatTable.find(*this);
    if(it != vertexFormatTable.end())
    {
        return it->second;
    }
    Console::warnf("Unsupported vertex attribute format: {}", ToString());
    return VK_FORMAT_UNDEFINED;
}

} // namespace graphics