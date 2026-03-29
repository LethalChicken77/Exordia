#pragma once
#include "core/asset_data.hpp"

namespace core
{

// Material asset containing material data and shader references
class MaterialAsset : AssetData
{
    public:
        MaterialAsset(const MaterialAsset&) = delete;
        MaterialAsset& operator=(const MaterialAsset&) = delete;
        MaterialAsset(MaterialAsset&&) = delete;
        MaterialAsset& operator=(MaterialAsset&&) = delete;

        // static std::unique_ptr<Scene> Instantiate(std::string name = "New Asset")
        // {
        //     std::unique_ptr<Scene> parent = Object::Instantiate<Scene>(name);
        //     return std::move(parent); // TODO: Put somewhere
        // }

        void drawScene();
        
    private:
        MaterialAsset(id_t newID) : AssetData(newID) {}

        uint64_t shaderGUID;

        // std::vector<GameObject> gameObjects;
};

} // namespace core