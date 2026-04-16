#pragma once
#include "shared_config.hpp"

namespace graphics
{
// API Agnostic shader info
enum class ShaderType
{
    Lit = 0,
    Unlit = 1,
    SpriteUnlit = 2,
    SpriteLit = 3,
    PostProcessing = 4,
    UI = 5
};

enum class DrawMode
{
    Fill = 0,
    Wireframe = 1,
    Points = 2
};

enum class BlendMode
{
    Opaque = 0,
    Alpha = 1,
    Additive = 2,
    Multiply = 3,
    Premultiply = 4
};

enum class CullMode
{
    None = 0,
    Front = 1,
    Back = 2,
    Both = 3
};

enum class DepthWrite
{
    Auto = 0,
    Enabled = 1,
    Disabled = 2
};

enum class StencilOp
{
    Keep = 0,
    Zero = 1,
    Replace = 2,
    IncrementAndClamp = 3,
    DecrementAndClamp = 4,
    Invert = 5,
    IncrementAndWrap = 6,
    DecrementAndWrap = 7
};

struct ShaderProperties
{
    ShaderType shaderType = ShaderType::Lit;
    DrawMode drawMode = DrawMode::Fill;
    BlendMode blendMode = BlendMode::Alpha;
    CullMode cullMode = CullMode::Back;
    CompareOp depthTest = CompareOp::LessEqual;
    DepthWrite depthWrite = DepthWrite::Auto;

    bool alphaClipping = false;
    float alphaClippingThreshold = 0.5f;

    // Additional settings
};

} // namespace graphics