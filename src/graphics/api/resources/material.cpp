#include "material.hpp"
#include "texture_data.hpp"

using graphics::ShaderParameter;
using graphics::BufferLayout;

namespace graphics
{

Material::Material(const Shader *shader) 
    // : shader(_shader), 
    : materialLayout(shader->GetLayoutPtr())
{
    shaderHandle = shader->graphicsHandle;
    if(materialLayout == nullptr) 
    {
        data = std::vector<uint8_t>(0);
        return;
    }

    const graphics::BufferLayout* materialInfoLayout = materialLayout->GetMaterialLayout();
    if(materialInfoLayout != nullptr)
    {
        // assert(materialLayout != nullptr && "Cannot create material if shader has no materialInfo field");
        data = std::vector<uint8_t>(materialInfoLayout->GetSize());
        for(const ShaderParameter &param : materialInfoLayout->GetParameters())
        {
            dataIndex[param.name] = param.offset;
        }
    }
    const ShaderLayout::DescriptorSetInfo* setInfo = materialLayout->GetMaterialDescriptorSet();
    Console::log("Creating material");
    if(setInfo != nullptr)
    {
        for(const ShaderLayout::BindingInfo &bindingInfo : setInfo->bindings)
        {
            if(bindingInfo.type == ShaderLayout::BindingType::CombinedImageSampler ||
                bindingInfo.type == ShaderLayout::BindingType::SampledImage ||
                bindingInfo.type == ShaderLayout::BindingType::StorageImage) // TODO: Figure out if I should handle texel buffers here
            {
                textureIndex.insert_or_assign(bindingInfo.name, textureBindings.size());
                textureBindings.emplace_back(TextureBinding(bindingInfo.binding, TextureHandle()));
            }
        }
    }
}

void Material::SetTexture(const std::string &name, TextureHandle handle)
{
    if(!textureIndex.contains(name))
    {
        Console::errorf("Material has no texture named {}.", name, "Material");
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
