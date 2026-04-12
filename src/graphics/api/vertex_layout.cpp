#include "vertex_layout.hpp"
#include "graphics/backend/vulkan_include.h"
#include "console.hpp"
#include <string>
#include <sstream>

namespace graphics
{

using AT = VertexLayout::AttrType;

uint32_t VertexLayout::Attribute::GetFormat() const
{
    
        // AttrType type = AttrType::Float; // Datatype of the attribute. Used for format selection.
        // uint8_t componentSize = 4;
        // uint8_t componentCount = 4;
        // uint8_t arrayCount = 1; // Used for matrix types.
    static const std::unordered_map<VertexLayout::AttrFormat, VkFormat, VertexLayout::AttrFormat::Hash> vertexFormatTable
    {
        {{AT::UInt,  1, 1, 1}, VK_FORMAT_R8_UINT},
        {{AT::SInt,  1, 1, 1}, VK_FORMAT_R8_SINT},
        {{AT::UNorm, 1, 1, 1}, VK_FORMAT_R8_UNORM},
        {{AT::SNorm, 1, 1, 1}, VK_FORMAT_R8_SNORM},
        {{AT::UInt,  1, 2, 1}, VK_FORMAT_R8G8_UINT},
        {{AT::SInt,  1, 2, 1}, VK_FORMAT_R8G8_SINT},
        {{AT::UNorm, 1, 2, 1}, VK_FORMAT_R8G8_UNORM},
        {{AT::SNorm, 1, 2, 1}, VK_FORMAT_R8G8_SNORM},
        {{AT::UInt,  1, 3, 1}, VK_FORMAT_R8G8B8_UINT},
        {{AT::SInt,  1, 3, 1}, VK_FORMAT_R8G8B8_SINT},
        {{AT::UNorm, 1, 3, 1}, VK_FORMAT_R8G8B8_UNORM},
        {{AT::SNorm, 1, 3, 1}, VK_FORMAT_R8G8B8_SNORM},
        {{AT::UInt,  1, 4, 1}, VK_FORMAT_R8G8B8A8_UINT},
        {{AT::SInt,  1, 4, 1}, VK_FORMAT_R8G8B8A8_SINT},
        {{AT::UNorm, 1, 4, 1}, VK_FORMAT_R8G8B8A8_UNORM},
        {{AT::SNorm, 1, 4, 1}, VK_FORMAT_R8G8B8A8_SNORM},

        {{AT::UInt,  2, 1, 1}, VK_FORMAT_R16_UINT},
        {{AT::SInt,  2, 1, 1}, VK_FORMAT_R16_SINT},
        {{AT::UNorm, 2, 1, 1}, VK_FORMAT_R16_UNORM},
        {{AT::SNorm, 2, 1, 1}, VK_FORMAT_R16_SNORM},
        {{AT::Float, 2, 1, 1}, VK_FORMAT_R16_SFLOAT},
        {{AT::UInt,  2, 2, 1}, VK_FORMAT_R16G16_UINT},
        {{AT::SInt,  2, 2, 1}, VK_FORMAT_R16G16_SINT},
        {{AT::UNorm, 2, 2, 1}, VK_FORMAT_R16G16_UNORM},
        {{AT::SNorm, 2, 2, 1}, VK_FORMAT_R16G16_SNORM},
        {{AT::Float, 2, 2, 1}, VK_FORMAT_R16G16_SFLOAT},
        {{AT::UInt,  2, 3, 1}, VK_FORMAT_R16G16B16_UINT},
        {{AT::SInt,  2, 3, 1}, VK_FORMAT_R16G16B16_SINT},
        {{AT::UNorm, 2, 3, 1}, VK_FORMAT_R16G16B16_UNORM},
        {{AT::SNorm, 2, 3, 1}, VK_FORMAT_R16G16B16_SNORM},
        {{AT::Float, 2, 3, 1}, VK_FORMAT_R16G16B16_SFLOAT},
        {{AT::UInt,  2, 4, 1}, VK_FORMAT_R16G16B16A16_UINT},
        {{AT::SInt,  2, 4, 1}, VK_FORMAT_R16G16B16A16_SINT},
        {{AT::UNorm, 2, 4, 1}, VK_FORMAT_R16G16B16A16_UNORM},
        {{AT::SNorm, 2, 4, 1}, VK_FORMAT_R16G16B16A16_SNORM},
        {{AT::Float, 2, 4, 1}, VK_FORMAT_R16G16B16A16_SFLOAT},

        {{AT::UInt,  4, 1, 1}, VK_FORMAT_R32_UINT},
        {{AT::SInt,  4, 1, 1}, VK_FORMAT_R32_SINT},
        {{AT::Float, 4, 1, 1}, VK_FORMAT_R32_SFLOAT},
        {{AT::UInt,  4, 2, 1}, VK_FORMAT_R32G32_UINT},
        {{AT::SInt,  4, 2, 1}, VK_FORMAT_R32G32_SINT},
        {{AT::Float, 4, 2, 1}, VK_FORMAT_R32G32_SFLOAT},
        {{AT::UInt,  4, 3, 1}, VK_FORMAT_R32G32B32_UINT},
        {{AT::SInt,  4, 3, 1}, VK_FORMAT_R32G32B32_SINT},
        {{AT::Float, 4, 3, 1}, VK_FORMAT_R32G32B32_SFLOAT},
        {{AT::UInt,  4, 4, 1}, VK_FORMAT_R32G32B32A32_UINT},
        {{AT::SInt,  4, 4, 1}, VK_FORMAT_R32G32B32A32_SINT},
        {{AT::Float, 4, 4, 1}, VK_FORMAT_R32G32B32A32_SFLOAT},



        {{AT::Float, 2, 1}, VK_FORMAT_R16_SFLOAT},
    };

    if(vertexFormatTable.contains(format))
    {
        return vertexFormatTable.at(format);
    }
    Console::warnf("Unsupported vertex attribute format: {}", format.ToString());
    return VK_FORMAT_UNDEFINED;
}


std::string VertexLayout::AttrFormat::ToString() const
{
    std::stringstream result{};
    result << "{";
    switch(type)
    {
        case AttrType::Invalid:
            result << "AttrType::Invalid, \t";
            break;
        case AttrType::UInt:
            result << "AttrType::UInt, \t";
            break;
        case AttrType::SInt:
            result << "AttrType::SInt, \t";
            break;
        case AttrType::UNorm:
            result << "AttrType::UNorm, \t";
            break;
        case AttrType::SNorm:
            result << "AttrType::SNorm, \t";
            break;
        case AttrType::Float:
            result << "AttrType::Float, \t";
            break;
    }
    result << std::format("CSize: {}, \tCCount: {}, \tACount: {}}}", componentSize, componentCount, arrayCount);
    return result.str();
}

std::string VertexLayout::Attribute::ToString() const
{
    std::stringstream result{};
    result << std::format("{{Location: {} \tOffset: {} \tSemantic: ",
        location,
        offset
    );
    switch(semantic)
    {
        case AttrSemantic::Other:
            result << "Other";
            break;
        case AttrSemantic::Position:
            result << "Position";
            break;
        case AttrSemantic::Normal:
            result << "Normal";
            break;
        case AttrSemantic::Color:
            result << "Color";
            break;
        case AttrSemantic::Tangent:
            result << "Tangent";
            break;
        case AttrSemantic::Bitangent:
            result << "Bitangent";
            break;
        case AttrSemantic::UV:
            result << "UV   ";
            break;
        case AttrSemantic::I_Model:
            result << "I_Model";
            break;
    }
    result << " \tFormat: " << format.ToString() << "}";
    return result.str();
}

std::string VertexLayout::ToString() const
{
    std::stringstream result{};
    result << "Vertex Layout:\n";
    for(const Attribute& attr : vertexAttributes)
    {
        result << "\t" << attr.ToString() << "\n";
    }
    result << std::format("\tStride: {}", GetStride());
    return result.str();
}

} // namespace graphics