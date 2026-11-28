#pragma once
#include "core/asset.hpp"
#include "graphics/api/resources/texture_data.hpp"
#include "graphics/api/resources/sprite.hpp"

namespace graphics
{
class TextureAsset : public core::Asset
{
public:
    static constexpr const std::string_view assetTypeName = "Texture Asset";

    void Load() override;
    void Unload() override;

    [[nodiscard]] inline Ref<TextureData> Texture() { return texture; };
    const Sprite* GetSprite(std::string_view name) const;

private:
    Ref<TextureData> texture{};
    std::vector<Sprite> sprites{};

    friend class core::AssetManager;
};
using TextureAssetRef = Ref<TextureAsset>;

} // namespace graphics