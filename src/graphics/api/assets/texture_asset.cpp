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


} // namespace graphics