#pragma once
#include <memory>
#include "graphics/api.hpp"

namespace core
{

class GameData
{
public:
    std::vector<graphics::Material> materials{};
};

extern std::unique_ptr<GameData> gameData;

}