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
    m_fileData = FileUtil::ReadFile<std::vector<char>>(m_path);
    m_fileLoaded = true;
    return true;
}

void Asset::unloadFile()
{
    m_fileData.clear();
    m_fileData.shrink_to_fit(); 
    m_fileLoaded = false;
}

} // namespace core