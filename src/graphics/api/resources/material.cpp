#include "material.hpp"
#include "texture_data.hpp"

using graphics::ShaderParameter;
using graphics::BufferLayout;

namespace graphics
{

Material::Material(const Shader *_shader) 
    : shader(_shader)
{
    Update();
}

/// @brief Assign a texture to a material via a texture handle.
/// @param name Name of the texture.
/// @param handle Texture handle.
void Material::SetTexture(const std::string &name, TextureHandle handle)
{
    if(!textureIndex.contains(name))
    {
        Console::errorf("Material has no texture named {}.", name);
        return;
    }
    if(!handle.IsValid())
    {
        Console::error("Cannot assign invalid texture handle to material.");
        return;
    }

    textureBindings[textureIndex.at(name)].handle = handle;
}

void Material::SetTexture(const std::string &name, const TextureData& texture)
{ 
    SetTexture(name, texture.graphicsHandle); 
}

/// @brief Create data buffers and index.
/// Migrates old values if possible.
/// @note Values will only be migrated if both the name and type match the old version.
void Material::Update()
{
    materialLayout = shader->GetLayoutPtr();
    std::vector<std::byte> oldData = data;
    std::unordered_map<std::string, DataIndex> oldDataMap = dataMap;
    data.clear();
    dataMap.clear();
    if(materialLayout == nullptr) 
    {
        return;
    }

    const graphics::BufferLayout* materialInfoLayout = materialLayout->GetMaterialLayout();
    if(materialInfoLayout != nullptr)
    {
        // assert(materialLayout != nullptr && "Cannot create material if shader has no materialInfo field");
        data = std::vector<std::byte>(materialInfoLayout->GetSize());
        for(const ShaderParameter &param : materialInfoLayout->GetParameters())
        {
            // Add new index
            DataIndex newIndex{};
            newIndex.index = param.offset;
            newIndex.type = param.type;
            dataMap.insert_or_assign(param.name, newIndex);
            // Remap old value
            auto it = oldDataMap.find(param.name);
            if(it != oldDataMap.end())
            {
                DataIndex oldIndex = it->second;
                // TODO: Handle applicable type conversions.
                if(oldIndex.type == newIndex.type)
                    memcpy(&data[newIndex.index], &oldData[oldIndex.index], newIndex.type.GetSize());
            }
        }
    }


    std::vector<TextureBinding> oldTexBindings = textureBindings;
    std::unordered_map<std::string, uint32_t> oldTexMap = textureIndex;
    textureBindings.clear();
    textureIndex.clear();
    const ShaderLayout::DescriptorSetInfo* setInfo = materialLayout->GetMaterialDescriptorSet();
    if(setInfo != nullptr)
    {
        for(const ShaderLayout::BindingInfo &bindingInfo : setInfo->bindings)
        {
            if(bindingInfo.type == ShaderLayout::BindingType::CombinedImageSampler ||
                bindingInfo.type == ShaderLayout::BindingType::SampledImage ||
                bindingInfo.type == ShaderLayout::BindingType::StorageImage) // TODO: Figure out if I should handle texel buffers here
            {
                textureIndex.insert_or_assign(bindingInfo.name, textureBindings.size());
                auto it = oldTexMap.find(bindingInfo.name);
                if(it != oldTexMap.end())
                {
                    uint32_t oldIndex = it->second;
                    textureBindings.emplace_back(TextureBinding(bindingInfo.binding, oldTexBindings[oldIndex].handle));
                }
                else
                {
                    textureBindings.emplace_back(TextureBinding(bindingInfo.binding, TextureHandle()));
                }
            }
        }
    }
}

bool Material::validateLayout() const noexcept
{
    if(materialLayout == nullptr)
    {
        Console::warnf("No material layout info found for material {}", name);
        return false;
    }
    if(materialLayout->GetMaterialLayout() == nullptr)
    {
        Console::warnf("No material buffer layout info found for material {}", name);
        return false;
    }
    return true;
}

const ShaderParameter* Material::getShaderParameter(const std::string& paramName, const BufferLayout* materialInfoLayout)
{
    const graphics::ShaderParameter* param = materialInfoLayout->GetParameter(paramName);
    if(param == nullptr)
    {
        Console::warnf("Shader layout for material {} has no parameter named {}", name, paramName);
        return nullptr;
    }
    if(!dataMap.contains(paramName))
    {
        // Console::warnf("Shader layout for material {} has changed, updating layout.", name, paramName);
        Update();
        return nullptr;
    }
    return param;
}


} // namespace core
