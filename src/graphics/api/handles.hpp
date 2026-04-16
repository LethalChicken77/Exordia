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
    void Invalidate() { *this = GHandle<Tag>(); }

    bool operator==(const GHandle& other) const
    {
        return other.index == index && other.generation == generation;
    }

    struct Hash
    {
        uint64_t operator()(const GHandle& handle) const
        {
            return static_cast<uint64_t>(handle.index) << 32 | handle.generation;
        }
    };
};

// Defined in a CPP to prevent outside usage
struct ShaderTag;
struct MaterialTag;
struct MeshTag;
struct TextureTag;

using ShaderHandle = GHandle<ShaderTag>;
using PipelineHandle = ShaderHandle;
using MaterialHandle = GHandle<MaterialTag>;
using MeshHandle = GHandle<MeshTag>;
using TextureHandle = GHandle<TextureTag>;

}