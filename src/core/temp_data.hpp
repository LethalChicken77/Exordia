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
    std::vector<graphics::Material> materials{};
    MemoryPool<graphics::Shader> shaderPool{64};
    Mesh skyboxMesh;
};

extern std::unique_ptr<GameData> gameData;

}