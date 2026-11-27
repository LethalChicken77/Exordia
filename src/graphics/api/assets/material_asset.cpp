#include "material_asset.hpp"
#include "TuiScript.h"
#include "graphics/api/assets/texture_asset.hpp"
#include "graphics/api/resources/material.hpp"
#include "graphics/resources/layouts/shader_layout.hpp"
#include "graphics/resources/layouts/var_layout.hpp"
#include "tui_helpers.hpp"
#include "asset.hpp"
#include "core/asset_manager.hpp"


namespace graphics
{

void parseMaterialValues(tui::table table, const ShaderLayout& layout, Material material) {
    tui::table materialInfoTable = table->getTable("materialInfo");
    if(!materialInfoTable)
        return;

    const BufferLayout *materialLayout = layout.GetMaterialLayout();
    if(materialLayout == nullptr) {
        // No material layout found
        return;
    }
    for(const ShaderParameter& param : materialLayout->GetParameters()) {
        if(param.type.type.IsBool()) {
            bool value = false;
            if(TuiHelpers::ReadBool(materialInfoTable, param.name.c_str(), value))
            {
                material.SetBool(param.name, value);
            }
        }
        else if(param.type.type.IsIntegral()) {
            if(param.type.componentCount == 1)
            {
                int64_t value = 0;
                if(TuiHelpers::ReadInt(materialInfoTable, param.name.c_str(), value))
                    material.SetInt(param.name, value);
            }
            else
            {
                switch(param.type.componentCount)
                {
                    case 2:
                    {
                        glm::ivec2 value;
                        if(TuiHelpers::ReadIVec2(materialInfoTable, param.name.c_str(), value))
                            material.SetVector(param.name, value);
                        break;
                    }
                    case 3:
                    {
                        glm::ivec3 value;
                        if(TuiHelpers::ReadIVec3(materialInfoTable, param.name.c_str(), value))
                            material.SetVector(param.name, value);
                        break;
                    }
                    case 4:
                    {
                        glm::ivec4 value;
                        if(TuiHelpers::ReadIVec4(materialInfoTable, param.name.c_str(), value))
                            material.SetVector(param.name, value);
                        break;
                    }
                }
            }
        }
        else if(param.type.type.IsFloat())
        {
            if(param.type.componentCount == 1)
            {
                float value = 0.0f;
                if(TuiHelpers::ReadFloat(materialInfoTable, param.name.c_str(), value))
                    material.SetFloat(param.name, value);
            }
            else
            {
                switch(param.type.componentCount)
                {
                    case 2:
                    {
                        glm::vec2 value;
                        if(TuiHelpers::ReadVec2(materialInfoTable, param.name.c_str(), value))
                            material.SetVector(param.name, value);
                        break;
                    }
                    case 3:
                    {
                        glm::vec3 value;
                        if(TuiHelpers::ReadVec3(materialInfoTable, param.name.c_str(), value))
                            material.SetVector(param.name, value);
                        break;
                    }
                    case 4:
                    {
                        glm::vec4 value;
                        if(TuiHelpers::ReadVec4(materialInfoTable, param.name.c_str(), value))
                            material.SetVector(param.name, value);
                        break;
                    }
                }
            }
        }
    }

    const ShaderLayout::DescriptorSetInfo* setInfo = layout.GetMaterialDescriptorSet();
    if(setInfo == nullptr)
        return;
    for(const ShaderLayout::BindingInfo &bindingInfo : setInfo->bindings)
    {
        if(bindingInfo.type != ShaderLayout::BindingType::CombinedImageSampler &&
            bindingInfo.type != ShaderLayout::BindingType::SampledImage &&
            bindingInfo.type != ShaderLayout::BindingType::StorageImage)
            continue;

        std::string texturePath = table->getString(bindingInfo.name);
        if(texturePath.empty())
            continue;

        Ref<TextureAsset> texAsset = core::AssetManager::LoadAsset<TextureAsset>(texturePath);
        if(!texAsset || !texAsset->IsReady())
            continue;

        material.SetTexture(bindingInfo.name, texAsset->Texture()->graphicsHandle);
    }
}

void MaterialAsset::Load() {
    m_status = core::AssetStatus::Loading;
    if(!loadFile()) {
        invalidate();
        return;
    }

    std::string dataString = std::string(DataString());
    tui::table table = tui::toTable(TuiRef::loadString(dataString, "loadString"));
    
    std::string shaderPath = table->getString("shader");
    shader = core::AssetManager::LoadAsset<ShaderAsset>(shaderPath);
    if(!shader || !shader->IsValid())
    {
        invalidate("Shader " + shaderPath + " is invalid.");
        return;
    }
    material = std::make_shared<MaterialData>(shader);
    material->name = std::filesystem::path(Path()).stem().string();

    parseMaterialValues(table, shader->GetLayout(), material);

    material.UpdateValues();

    m_status = core::AssetStatus::Ready;
    shader->Unload();
    unloadFile();
}

void MaterialAsset::Unload() {
    material.reset();
    shader.reset();
    unloadFile();
}


} // namespace graphics