#include "material.hpp"

using graphics::ShaderParameter;
using graphics::BufferLayout;

namespace graphics
{

Material::Material(const Shader *_shader) 
    : shader(_shader), 
    materialLayout(shader->GetLayout().GetMaterialLayout())
{
    if(materialLayout == nullptr) 
    {
        data = std::vector<uint8_t>(16);
        return;
    }
    // assert(materialLayout != nullptr && "Cannot create material if shader has no materialInfo field");
    data = std::vector<uint8_t>(materialLayout->GetSize());
    for(const ShaderParameter &param : materialLayout->GetParameters())
    {
        dataIndex[param.name] = param.offset;
    }
}

} // namespace core
