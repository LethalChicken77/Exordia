#include "material.hpp"

using graphics::ShaderParameter;
using graphics::ShaderLayout;

namespace core
{

Material::Material(const Shader *_shader) : shader(_shader)
{
    data = std::vector<uint8_t>(shader->GetLayout().GetSize());
    for(const ShaderParameter &param : shader->GetLayout().GetParameters())
    {
        dataIndex[param.name] = param.offset;
    }
}

} // namespace core
