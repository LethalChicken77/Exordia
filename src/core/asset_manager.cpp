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
} // namespace core