#pragma once
#include <cstdint>
#include <cassert>

namespace graphics::Alignment
{

/// @brief Align the offset up to alignment. Assumes alignment is a power of 2.
/// @param offset 
/// @param alignment 
/// @return Aligned offset.
inline uint32_t AlignUp(uint32_t offset, uint32_t alignment)
{
    assert(alignment != 0 && (alignment & (alignment - 1)) == 0);
    return (offset + (alignment - 1)) & ~(alignment - 1);
}

/// @brief Align the offset down to alignment. Assumes alignment is a power of 2.
/// @param offset 
/// @param alignment 
/// @return Aligned offset.
/// @note This might be useless, you probably want AlignUp.
inline uint32_t AlignDown(uint32_t offset, uint32_t alignment)
{
    assert(alignment != 0 && (alignment & (alignment - 1)) == 0);
    return offset & ~(alignment - 1);
}

};