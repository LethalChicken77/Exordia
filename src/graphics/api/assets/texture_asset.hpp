#pragma once
#include "core/asset.hpp"
#include "graphics/api/resources/texture_data.hpp"

namespace graphics
{
class TextureAsset : public core::Asset
{
public:
    static constexpr const std::string_view assetTypeName = "Texture Asset";

    void Load() override;
    void Unload() override;

    [[nodiscard]] inline Ref<TextureData> Texture() { return texture; };

private:
    Ref<TextureData> texture{};

    friend class core::AssetManager;
};
using TextureAssetRef = Ref<TextureAsset>;

} // namespace graphics