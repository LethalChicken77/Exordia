#include "asset_manager.hpp"

#include "file_util.hpp"
#include "console.hpp"

namespace core
{
bool AssetManager::fileCheck(std::filesystem::path path)
{
    if(!FileUtil::FileExists(path))
    {
        Console::error("File not found: " + path.string(), "Asset Manager");
        return false;
    }
    return true;
}

void AssetManager::printLoading(std::string_view typeName, std::string_view path)
{
    Console::logf("Loading {} {}", typeName, path);
}
} // namespace core