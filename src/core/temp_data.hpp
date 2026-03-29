#pragma once
#include <memory>
#include "graphics/api.hpp"
#include "pool_allocator.hpp"
#include "mesh.hpp"

namespace core
{

class GameData
{
public:
    MemoryPool<graphics::Material> materials{1024};
    MemoryPool<graphics::Shader> shaderPool{64};
    MemoryPool<graphics::TextureData> textures{1024};
    Mesh skyboxMesh;
    graphics::Material* skyboxMaterial;
};

extern std::unique_ptr<GameData> gameData;

}