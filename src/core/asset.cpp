#include "asset.hpp"
#include "file_util.hpp"
#include "console.hpp"

namespace core
{
Asset::~Asset()
{
    Unload();
}

void Asset::Load()
{
    loadFile();
}

void Asset::Unload()
{
    unloadFile();
}

bool Asset::loadFile()
{
    if(!FileUtil::FileExists(m_path))
    {
        Console::error("File not found: " + m_path.string(), "AssetData");
        return false;
    }
    if(m_fileLoaded)
    {
        // Console::warnf("File {} already loaded.", m_path.string(), "AssetData");
        return true;
    }
    m_fileData = FileUtil::ReadFile<std::vector<char>>(m_path);
    m_fileLoaded = true;
    std::filesystem::path metaDataPath = m_path;
    metaDataPath.concat(".meta");
    if(FileUtil::FileExists(metaDataPath))
    {
        m_metaData = FileUtil::ReadFile<std::vector<char>>(metaDataPath);
    }
    return true;
}

void Asset::unloadFile()
{
    m_fileData.clear();
    m_fileData.shrink_to_fit(); 
    m_fileLoaded = false;
}

void Asset::invalidate()
{
    m_status = AssetStatus::Invalid;
}

void Asset::invalidate(std::string_view message)
{
    m_status = AssetStatus::Invalid;
    Console::error(message, assetTypeName);
}

} // namespace core