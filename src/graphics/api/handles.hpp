#pragma once
#include <cstdint>

namespace graphics
{
// All have the same fields for now, they are defined separately to help prevent the incorrect handle being passed to a function

template<typename Tag>
struct GHandle
{
    uint32_t index = ~0u; // Index into array
    uint32_t generation = 0; // UAF protection

    bool IsValid() const { return index != ~0u; }
};

// Defined in a CPP to prevent outside usage
struct ShaderTag;
struct MaterialTag;
struct MeshTag;
struct TextureTag;

using ShaderHandle = GHandle<ShaderTag>;
using MaterialHandle = GHandle<MaterialTag>;
using MeshHandle = GHandle<MeshTag>;
using TextureHandle = GHandle<TextureTag>;

}