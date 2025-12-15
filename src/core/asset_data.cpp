#include "asset_data.hpp"
#include "utils/file_util.hpp"
#include "utils/console.hpp"

namespace core
{
void AssetData::LoadData()
{
    if(!FileUtil::fileExists(path))
    {
        Console::error("File not found: " + path, "AssetData");
        return;
    }
    data = FileUtil::readFileToCharVector(path);
}
} // namespace core