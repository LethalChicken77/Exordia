#pragma once
#include <memory>
#include "graphics/api.hpp"
#include "pool_allocator.hpp"
#include "core/resources/mesh.hpp"

namespace core
{

class GameData
{
public:
    MemoryPool<graphics::Shader> shaderPool{64};
    MemoryPool<graphics::TextureData> textures{1024};
    Mesh skyboxMesh;
    graphics::MaterialAssetRef yellowMaterial{};
    graphics::MaterialAssetRef blueMaterial{};
    graphics::MaterialAssetRef groundMaterial{};
    graphics::MaterialAssetRef skyboxMaterial{};
    graphics::MaterialAssetRef spriteMaterial{};
    graphics::MaterialAssetRef spriteTiledMaterial{};
};

extern std::unique_ptr<GameData> gameData;

}