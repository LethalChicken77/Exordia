#pragma once
#include "shared_config.hpp"

namespace graphics
{
// API Agnostic shader info
enum class ShaderType
{
    LIT = 0,
    UNLIT = 1,
    SPRITE_UNLIT = 2,
    SPRITE_LIT = 3,
    POST_PROCESSING = 4,
    UI = 5
};

enum class DrawMode
{
    FILL = 0,
    WIREFRAME = 1,
    POINTS = 2
};

enum class BlendMode
{
    OPAQUE = 0,
    ALPHA = 1,
    ADDITIVE = 2,
    MULTIPLY = 3,
    PREMULTIPLY = 4
};

enum class CullMode
{
    NONE = 0,
    FRONT = 1,
    BACK = 2,
    BOTH = 3
};

enum class DepthWrite
{
    AUTO = 0,
    ENABLED = 1,
    DISABLED = 2
};

enum class StencilOp
{
    KEEP = 0,
    ZERO = 1,
    REPLACE = 2,
    INCREMENT_AND_CLAMP = 3,
    DECREMENT_AND_CLAMP = 4,
    INVERT = 5,
    INCREMENT_AND_WRAP = 6,
    DECREMENT_AND_WRAP = 7
};

struct ShaderProperties
{
    ShaderType shaderType = ShaderType::LIT;
    DrawMode drawMode = DrawMode::FILL;
    BlendMode blendMode = BlendMode::ALPHA;
    CullMode cullMode = CullMode::BACK;
    CompareOp depthTest = CompareOp::LessEqual;
    DepthWrite depthWrite = DepthWrite::AUTO;

    bool alphaClipping = false;
    float alphaClippingThreshold = 0.5f;

    // Additional settings
};

} // namespace graphics