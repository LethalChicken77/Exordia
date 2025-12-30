// Header only utilities for graphics module
#pragma once

#include <cstdint>

namespace graphics
{
    
constexpr uint32_t AlignedSize(uint32_t size, uint32_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

} // namespace graphics