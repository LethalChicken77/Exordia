#pragma once
#include "core/asset.hpp"
#include "graphics/api/resources/material.hpp"

namespace graphics
{
class MaterialAsset : public core::Asset
{
public:
    static constexpr const std::string_view assetTypeName = "Material Asset";

    void Load() override;
    void Unload() override;

    [[nodiscard]] inline Material GetMaterial() { return material; };
    [[nodiscard]] inline Shader GetShader() { return shader; };

private:

    Shader shader{};
    Material material{};

    friend class core::AssetManager;
};
using MaterialAssetRef = Ref<MaterialAsset>;

} // namespace graphics