#pragma once
#include <cstdint>

namespace graphics
{

enum class PrimitiveTopology : uint8_t
{
    TriangleList = 0,
    TriangleStrip = 1,
    TriangleFan = 2,

    LineList = 3,
    LineStrip = 4,

    PointList = 5
};

struct MeshConfig
{
    PrimitiveTopology primitiveTopology = PrimitiveTopology::TriangleList;
    bool enablePrimitiveRestart = false;
};

} // namespace graphics