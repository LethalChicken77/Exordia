#pragma once
#include <cstdint>

namespace graphics::PushConstants
{

struct Object
{
    uint32_t id;
    uint32_t flags;
};

struct Compute
{
    uint32_t dispatchID;
    uint32_t flags;
    float time;
    float dt;
};

} // namespace graphics::PushConstants