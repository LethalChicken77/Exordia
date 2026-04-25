#include "material.hpp"
#include "texture_data.hpp"
#include "imgui.h"
#include "imgui_extensions.hpp"
#include "modules.hpp"

using graphics::ShaderParameter;
using graphics::BufferLayout;

namespace graphics
{

Material::Material(const Shader *_shader, const std::string_view _name) 
    : shader(_shader),
    name(_name)
{
    UpdateLayout(false);
    if(materialLayout != nullptr)
    {
        const BufferLayout* bufferLayout = materialLayout->GetMaterialLayout();
        if(bufferLayout != nullptr)
        {
            for(const ShaderParameter& param : bufferLayout->GetParameters())
            {
                if(param.type.type.IsIntegral() && param.type.componentCount == 1)
                {
                    setIntegralParam<uint64_t>(&param, param.metaData.defaultInt);
                }
                if(param.type.type.IsFloat() && param.type.componentCount == 1)
                {
                    setFloatParam<float>(&param, param.metaData.defaultFloat);
                }
            }
        }
    }
    UpdateValues();
}

Material::~Material()
{
    graphicsModule.DeregisterMaterial(*this);
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
    UpdateValues();
}

void Material::SetTexture(const std::string &name, const TextureData& texture)
{ 
    SetTexture(name, texture.graphicsHandle); 
}

/// @brief Create data buffers and index.
/// Migrates old values if possible.
/// @note Values will only be migrated if both the name and type match the old version.
void Material::UpdateLayout(bool updateVals)
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

    if(updateVals)
        UpdateValues();
}

void Material::UpdateValues()
{
    // Console::debugf("Updating material: {}", name);
    graphicsModule.RegisterMaterial(*this);
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
        UpdateLayout();
        return nullptr;
    }
    return param;
}

ImGuiDataType typeToImguiTypeScalar(TypeDescription td)
{
    switch(td.type)
    {
        case DataType::UInt:
            switch(td.componentSize)
            {
                case 1:
                    return ImGuiDataType_U8;
                case 2:
                    return ImGuiDataType_U16;
                case 4:
                default:
                    return ImGuiDataType_U32;
                case 8:
                    return ImGuiDataType_U64;
            }
        case DataType::SInt:
            switch(td.componentSize)
            {
                case 1:
                    return ImGuiDataType_S8;
                case 2:
                    return ImGuiDataType_S16;
                case 4:
                default:
                    return ImGuiDataType_S32;
                case 8:
                    return ImGuiDataType_S64;
            }
        default:
        case DataType::Float:
            switch(td.componentSize)
            {
                case 2:
                    return ImGuiDataType_Float;
                case 4:
                default:
                    return ImGuiDataType_Float;
                case 8:
                    return ImGuiDataType_Double;
            }
    }
}

bool Material::drawImGuiParam(const ShaderParameter& param)
{
    std::byte* dest = data.data() + param.offset;
    const char* paramName = param.name.c_str();
    bool dirty = false;
    switch(param.type.type)
    {
    case DataType::Bool:
        dirty |= ImGui::Checkbox(paramName, reinterpret_cast<bool*>(dest));
        break;
    case DataType::UInt:
    case DataType::SInt:
    {
        long min = param.type.type == DataType::SInt ? INT32_MIN : 0;
        long max = param.type.type == DataType::SInt ? INT32_MAX : UINT32_MAX;
        void* minP = &min;
        void* maxP = &max;
        if(param.metaData.useMin)
        {
            min = param.metaData.min;
        }
        if(param.metaData.useMax)
        {
            max = param.metaData.max;
        }
        double test;
        
        // TODO: properly handle 8 bit colors
        // if(param.metaData.isColor && param.type.componentSize == 1 && param.type.componentCount == 3)
        // {
        //     dirty |= ImGui::ColorEdit3(paramName, reinterpret_cast<float*>(dest), ImGuiColorEditFlags_Uint8);
        // }
        // else if(param.metaData.isColor && param.type.componentSize == 1 && param.type.componentCount == 4)
        // {
        //     dirty |= ImGui::ColorEdit4(paramName, reinterpret_cast<float*>(dest), ImGuiColorEditFlags_Uint8);
        // }
        if(param.metaData.useMax && param.metaData.useMin)
        {
            dirty |= ImGui::SliderScalarN(paramName, typeToImguiTypeScalar(param.type), reinterpret_cast<void*>(dest), param.type.componentCount, &min, &max);
        }
        else
        {
            dirty |= ImGui::DragScalarN(paramName, typeToImguiTypeScalar(param.type), reinterpret_cast<void*>(dest), param.type.componentCount, 1.0f, &min, &max);
        }
        break;
    }
    case DataType::Float:
    {
        if(param.type.componentSize == 8)
        {
            double min = std::numeric_limits<double>::min();
            double max = std::numeric_limits<double>::max();
            if(param.metaData.useMin)
            {
                min = param.metaData.min;
            }
            if(param.metaData.useMax)
            {
                max = param.metaData.max;
            }

            if(param.metaData.useMax && param.metaData.useMin)
            {
                dirty |= ImGui::SliderDoubleN(paramName, reinterpret_cast<double*>(dest), param.type.componentCount, min, max);
            }
            else
            {
                dirty |= ImGui::DragDoubleN(paramName, reinterpret_cast<double*>(dest), param.type.componentCount, 0.01f, min, max);
            }
            break;
        }
        else
        {
            float tempFloats[param.type.componentCount];
            for(uint32_t i = 0; i < param.type.componentSize; i++)
            {
                tempFloats[i] = param.type.componentSize == 2 ?
                    UnpackHalfToFloat(reinterpret_cast<uint16_t*>(dest)[i]) :
                    reinterpret_cast<float*>(dest)[i];
            }
            float min = std::numeric_limits<float>::min();
            float max = std::numeric_limits<float>::max();
            float* minP = &min;
            float* maxP = &max;
            if(param.metaData.useMin)
            {
                min = param.metaData.min;
            }
            if(param.metaData.useMax)
            {
                max = param.metaData.max;
            }

            if(param.metaData.isColor && param.type.componentCount == 3)
            {
                dirty |= ImGui::ColorEdit3(paramName, tempFloats);
            }
            else if(param.metaData.isColor && param.type.componentCount == 4)
            {
                dirty |= ImGui::ColorEdit4(paramName, tempFloats);
            }
            else if(param.metaData.useMax && param.metaData.useMin)
            {
                dirty |= ImGui::SliderFloatN(paramName, tempFloats, param.type.componentCount, min, max);
            }
            else
            {
                dirty |= ImGui::DragFloatN(paramName, tempFloats, param.type.componentCount, 0.01f, min, max);
            }

            for(uint32_t i = 0; i < param.type.componentSize; i++)
            {
                if(param.type.componentSize == 2)
                    reinterpret_cast<uint16_t*>(dest)[i] = PackFloatToHalf(tempFloats[i]);
                else
                    reinterpret_cast<float*>(dest)[i] = tempFloats[i];
            }

            break;
        }
        break;
    }
    default:
        ImGui::Text("%s", param.name.c_str());
        break;
    }
    return dirty;
}

void Material::DrawImGui()
{
    ImGui::Text("%s", name.c_str());
    if(!validateLayout())
    {
        UpdateLayout();
        return;
    }
    if(materialLayout == nullptr) 
    {
        ImGui::Text("Material layout is null.");
        return;
    }
    bool dirty = false;
    for(const ShaderLayout::BindingInfo& binding : materialLayout->GetMaterialDescriptorSet()->bindings)
    {
        if(binding.type.IsBuffer())
        {
            const BufferLayout& buffLayout = materialLayout->GetBufferLayouts()[binding.bufferIndex];
            for(const ShaderParameter& param : buffLayout.GetParameters())
            {
                dirty |= drawImGuiParam(param);
            }
        }
        else if(binding.type.IsImage())
        {

        }
    }
    if(dirty) UpdateValues();
}

} // namespace core
