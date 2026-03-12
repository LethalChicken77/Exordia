#pragma once
#include <cstdint>

namespace graphics
{
// All have the same fields for now, they are defined separately to help prevent the incorrect handle being passed to a function

// Handle for the graphics data for a shader
struct ShaderHandle
{
    uint32_t index = ~0u; // Index of the graphics shader (GraphicsPipeline)
    uint32_t generation = 0; // UAF protection

    bool IsValid() const { return index != ~0u; }
};

// Handle for the graphics data for a material
struct MaterialHandle
{
    uint32_t index = ~0u; // Index of the graphics material (ShaderBuffer)
    uint32_t generation = 0; // UAF protection

    bool IsValid() const { return index != ~0u; }
};

// Handle for the graphics data for a mesh
struct MeshHandle
{
    uint32_t index = ~0u; // Index of the graphics mesh
    uint32_t generation = 0; // UAF protection

    bool IsValid() const { return index != ~0u; }
};

// Handle for the graphics data for a texture
struct TextureHandle
{
    uint32_t index = ~0u; // Index of the graphics texture
    uint32_t generation = 0; // UAF protection

    bool IsValid() const { return index != ~0u; }
};

}