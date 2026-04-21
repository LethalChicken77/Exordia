#include "vertex_layout.hpp"
#include "graphics/backend/vulkan_include.h"
#include "console.hpp"
#include <string>
#include <sstream>

namespace graphics
{

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
    vk::Format validationFormat = (vk::Format)GetVkFormat();
    result << " \tFormat: " << format.ToString() << "} : " << vk::to_string(validationFormat);
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