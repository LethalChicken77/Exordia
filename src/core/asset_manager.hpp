#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "asset.hpp"
#include "string_hash.hpp"

namespace core
{

class AssetManager
{
public:
    // TODO: CreateAsset
    
    /// @brief Load an asset and register it in the table
    template<std::derived_from<Asset> T>
    static Ref<T> LoadAsset(std::string_view path, bool reload = false)
    {
        Ref<T> asset = GetAsset<T>(path);
        if(asset != nullptr) // Asset already registered
        {
            if(reload)
            {
                asset->Reload();
            }
            return asset;
        }

        if(!fileCheck(path))
            return nullptr;

        asset = std::make_shared<T>();
        asset->SetPath(path);
        asset->Load();

        assetTable.try_emplace(std::string(path), asset);

        return asset;
    }

    /// @brief Retrieve an asset that already exists
    template<std::derived_from<Asset> T>
    static Ref<T> GetAsset(std::string_view path)
    {
        auto tableLookup = assetTable.find(path);
        if(tableLookup == assetTable.end())
        {
            return nullptr;
        }

        Ref<Asset> asset = tableLookup->second.lock();
        if(!asset)
        {
            return nullptr;
        }

        return std::dynamic_pointer_cast<T>(asset);
    }

    static Ref<Asset> GetAsset(std::string_view path)
    {
        auto tableLookup = assetTable.find(std::string(path));
        if(tableLookup == assetTable.end())
        {
            return nullptr;
        }

        return tableLookup->second.lock();
    }
private:
    inline static std::unordered_map<std::string, WRef<Asset>, StringHash, std::equal_to<>> assetTable{};
    static bool fileCheck(std::filesystem::path path);
};

} // namespace core