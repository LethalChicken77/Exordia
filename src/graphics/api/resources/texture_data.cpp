#include "texture_data.hpp"

#include <format>
#include "utils/console.hpp"

namespace graphics
{
// uint8_t TextureData::GetByte(uint32_t x, uint32_t y, uint32_t z) const
// {
//     #ifndef DISABLE_VALIDATION
//     if(x >= width || y >= height || z >= depth)
//     {
//         Console::warn(std::format("Cannot access pixel outside texture bounds: ({}, {}, {}), Size: ({}, {}, {}). Returning 0", x, y, z, width, height, depth), "TextureData");
//         return 0;
//     }
//     if(GetPixelSize() != 1)
//     {
//         Console::warn(std::format("Pixel size is not one byte. Data may be misinterpreted"), "TextureData");
//     }
//     #endif
//     uint32_t index = (z * width * height + y * width + x) * GetPixelSize();
//     if(index + 3 >= data.size())
//     {
//         throw std::out_of_range("TextureData get() index out of range");
//     }
//     return
//         (data[index] << 24) |
//         (data[index + 1] << 16) |
//         (data[index + 2] << 8) |
//         (data[index + 3]);
// }
} // namespace core