#include "vertex_layout.hpp"
#include "graphics/backend/vulkan_include.h"

namespace graphics
{

using AT = VertexLayout::AttributeType;
using AL = VertexLayout::AttributeLayout;

uint32_t VertexLayout::Attribute::GetFormat() const
{
    static const std::unordered_map<VertexLayout::Attribute, VkFormat, VertexLayout::Attribute::Hash> vertexFormatTable
    {
        // Position
        {{AT::Position, AL::Standard, 2}, VK_FORMAT_R32G32_SFLOAT},
        {{AT::Position, AL::Standard, 3}, VK_FORMAT_R32G32B32_SFLOAT},
        {{AT::Position, AL::Standard, 4}, VK_FORMAT_R32G32B32A32_SFLOAT},

        // Normal/Bitangent
        {{AT::Normal, AL::Standard, 3}, VK_FORMAT_R32G32B32_SFLOAT},
        {{AT::Normal, AL::Half, 3}, VK_FORMAT_R16G16B16_SFLOAT},
        {{AT::Normal, AL::Octahedral, 2}, VK_FORMAT_R8G8_SNORM},
        {{AT::Bitangent, AL::Standard, 3}, VK_FORMAT_R32G32B32_SFLOAT},
        {{AT::Bitangent, AL::Half, 3}, VK_FORMAT_R16G16B16_SFLOAT},
        {{AT::Bitangent, AL::Octahedral, 2}, VK_FORMAT_R8G8_SNORM},

        // Tangent
        {{AT::Tangent, AL::Standard, 4}, VK_FORMAT_R32G32B32A32_SFLOAT},
        {{AT::Tangent, AL::Half, 4}, VK_FORMAT_R16G16B16A16_SFLOAT},
        {{AT::Tangent, AL::Octahedral, 3}, VK_FORMAT_R8G8B8_SNORM},

        // Color
        {{AT::Color, AL::Standard, 3}, VK_FORMAT_R32G32B32_SFLOAT},
        {{AT::Color, AL::Standard, 4}, VK_FORMAT_R32G32B32A32_SFLOAT},
        {{AT::Color, AL::Half, 3}, VK_FORMAT_R16G16B16_SFLOAT},
        {{AT::Color, AL::Half, 4}, VK_FORMAT_R16G16B16A16_SFLOAT},
        {{AT::Color, AL::UInt8, 3}, VK_FORMAT_R8G8B8_UNORM},
        {{AT::Color, AL::UInt8, 4}, VK_FORMAT_R8G8B8A8_UNORM},

        // UV
        {{AT::UV, AL::Standard, 2}, VK_FORMAT_R32G32_SFLOAT},
        {{AT::UV, AL::Half, 2}, VK_FORMAT_R16G16_SFLOAT},

        // Other
        {{AT::Other, AL::Standard, 1}, VK_FORMAT_R32_SFLOAT},
        {{AT::Other, AL::Half, 1}, VK_FORMAT_R16_SFLOAT},
        {{AT::Other, AL::UInt8, 1}, VK_FORMAT_R8_UNORM},

        {{AT::Other, AL::Standard, 2}, VK_FORMAT_R32G32_SFLOAT},
        {{AT::Other, AL::Half, 2}, VK_FORMAT_R16G16_SFLOAT},
        {{AT::Other, AL::UInt8, 2}, VK_FORMAT_R8G8_UNORM},
        {{AT::Other, AL::Octahedral, 2}, VK_FORMAT_R8G8_SNORM},

        {{AT::Other, AL::Standard, 3}, VK_FORMAT_R32G32B32_SFLOAT},
        {{AT::Other, AL::Half, 3}, VK_FORMAT_R16G16B16_SFLOAT},
        {{AT::Other, AL::UInt8, 3}, VK_FORMAT_R8G8B8_UNORM},

        {{AT::Other, AL::Standard, 4}, VK_FORMAT_R32G32B32A32_SFLOAT},
        {{AT::Other, AL::Half, 4}, VK_FORMAT_R16G16B16A16_SFLOAT},
        {{AT::Other, AL::UInt8, 4}, VK_FORMAT_R8G8B8A8_UNORM},
    };
    if(vertexFormatTable.contains(*this))
    {
        return vertexFormatTable.at(*this);
    }
    return VK_FORMAT_UNDEFINED;
}

} // namespace graphics