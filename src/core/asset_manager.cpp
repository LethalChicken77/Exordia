#include "asset_manager.hpp"

namespace core
{
    std::unordered_map<id_t, AssetData*> AssetManager::assets{};
} // namespace core