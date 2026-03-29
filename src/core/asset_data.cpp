#include "asset_data.hpp"
#include "utils/file_util.hpp"
#include "utils/console.hpp"

namespace core
{
void AssetData::LoadData()
{
    if(!FileUtil::FileExists(path))
    {
        Console::error("File not found: " + path, "AssetData");
        return;
    }
    data = FileUtil::ReadFileToCharVector(path);
}
} // namespace core