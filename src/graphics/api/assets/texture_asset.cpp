#include "console.hpp"
#include "graphics/api/texture_properties.hpp"
#include "modules.hpp"
#include "texture_asset.hpp"
#include "file_util.hpp"
#include "graphics/resources/layouts/var_layout.hpp"
#include "asset.hpp"
#include "core/asset_manager.hpp"
#include <memory>
#include "TuiScript.h"
#include "tui_helpers.hpp"


namespace graphics
{

void parseProperties(tui::table table, TextureConfig& config)
{
    table = table->getTable("properties");
    if(!table)
        return;

    TuiHelpers::ReadBool(table, "generateMipmaps", config.generateMipmaps);

    tui::table swizTable = table->getTable("swizzle");
    if(swizTable)
    {
        TuiHelpers::ReadEnum(swizTable, "r", config.swizzle.r, SwizzleChannel::Red, SwizzleChannel::One);
        TuiHelpers::ReadEnum(swizTable, "g", config.swizzle.g, SwizzleChannel::Red, SwizzleChannel::One);
        TuiHelpers::ReadEnum(swizTable, "b", config.swizzle.b, SwizzleChannel::Red, SwizzleChannel::One);
        TuiHelpers::ReadEnum(swizTable, "a", config.swizzle.a, SwizzleChannel::Red, SwizzleChannel::One);
    }
    TuiHelpers::ReadEnum(table, "shape", config.shape, TextureShape::s1D, TextureShape::ArrayCube);

    TuiHelpers::ReadEnum(table, "wrapU", config.wrapU, TextureWrapMode::Repeat, TextureWrapMode::MirrorClampToEdge);
    TuiHelpers::ReadEnum(table, "wrapV", config.wrapV, TextureWrapMode::Repeat, TextureWrapMode::MirrorClampToEdge);
    TuiHelpers::ReadEnum(table, "wrapW", config.wrapW, TextureWrapMode::Repeat, TextureWrapMode::MirrorClampToEdge);

    TuiHelpers::ReadEnum(table, "magFilter", config.magFilter, TextureFilterMode::Nearest, TextureFilterMode::Cubic);
    TuiHelpers::ReadEnum(table, "minFilter", config.minFilter, TextureFilterMode::Nearest, TextureFilterMode::Cubic);
    TuiHelpers::ReadEnum(table, "mipmapMode", config.mipmapMode, MipmapMode::None, MipmapMode::Linear);

    TuiHelpers::ReadFloat(table, "mipLodBias", config.mipLodBias);

    TuiHelpers::ReadEnum(table, "borderColorMode", config.borderColorMode, BorderColorMode::FTransparentBlack, BorderColorMode::ICustom);
    TuiHelpers::ReadVec4(table, "borderColor", *config.borderColor.Vec4Ptr());

    TuiHelpers::ReadBool(table, "anisotropicFiltering", config.anisotropicFiltering);
    TuiHelpers::ReadFloat(table, "maxAnisotropy", config.maxAnisotropy);

    TuiHelpers::ReadBool(table, "compareEnable", config.compareEnable);
    TuiHelpers::ReadEnum(table, "compareOp", config.compareOp, CompareOp::Never, CompareOp::Always);

    TuiHelpers::ReadFloat(table, "minLod", config.minLod);
    TuiHelpers::ReadFloat(table, "maxLod", config.maxLod);

    TuiHelpers::ReadBool(table, "srgb", config.srgb);
    TuiHelpers::ReadBool(table, "readWrite", config.readWrite);

    TuiHelpers::ReadFloat(table, "pixelsPerUnit", config.pixelsPerUnit);
}

void TextureAsset::Load() {
    m_status = core::AssetStatus::Loading;
    if(!loadFile()) {
        invalidate();
        return;
    }

    texture = std::make_shared<TextureData>();
    FileType type = FileUtil::IdentifyFileType(Path(), Data());
    switch(type)
    {
    case FileType::Png:
    case FileType::Jpeg:
        texture->LoadSTB(Data<uint8_t>());
        break;
    case FileType::Exr:
        texture->LoadEXR(Data<uint8_t>());
        break;
    default:
        invalidate(std::format("Unsupported image type {}", Path()));
        return;
    }

    if(MetadataString().length() > 0)
    {
        std::string dataString = std::string(MetadataString());
        tui::table table = tui::toTable(TuiRef::loadString(dataString, "loadString"));
        // Load texture properties
        if(table)
            parseProperties(table, texture->properties);
    }

    graphicsModule->RegisterTexture(texture);
    
    m_status = core::AssetStatus::Ready;
    unloadFile();
}

void TextureAsset::Unload() {
    graphicsModule->DeregisterTexture(texture);
    texture.reset();
    unloadFile();
}

const Sprite* TextureAsset::GetSprite(std::string_view name) const
{
    for(const Sprite& sprite : sprites)
    {
        if(sprite.name == name)
        {
            return &sprite;
        }
    }
    return nullptr;
}


} // namespace graphics