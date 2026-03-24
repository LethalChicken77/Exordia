#include "material.hpp"
#include "texture_data.hpp"

using graphics::ShaderParameter;
using graphics::BufferLayout;

namespace graphics
{

Material::Material(const Shader *shader) 
    // : shader(_shader), 
    : materialLayout(shader->GetLayout().GetMaterialLayout())
{
    shaderHandle = shader->graphicsHandle;
    if(materialLayout == nullptr) 
    {
        data = std::vector<uint8_t>(0);
        return;
    }
    // assert(materialLayout != nullptr && "Cannot create material if shader has no materialInfo field");
    data = std::vector<uint8_t>(materialLayout->GetSize());
    for(const ShaderParameter &param : materialLayout->GetParameters())
    {
        dataIndex[param.name] = param.offset;
    }
}

void Material::SetTexture(const std::string &name, TextureHandle handle)
{
    if(!textureIndex.contains(name))
    {
        Console::errorf("Material does not contain texture named {}.", name, "Material");
        return;
    }
    if(!handle.IsValid())
    {
        Console::error("Cannot assign invalid texture handle to material.", "Material");
        return;
    }

    textureBindings[textureIndex.at(name)].handle = handle;
}

void Material::SetTexture(const std::string &name, const TextureData& texture)
{ SetTexture(name, texture.graphicsHandle); }

} // namespace core
