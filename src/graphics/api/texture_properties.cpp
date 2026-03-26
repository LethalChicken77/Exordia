#include "texture_properties.hpp"
#include <format>

namespace graphics
{

std::string ImageFormat::ToString() const
{
    return std::format("ImageFormat{{SRGB: {}\tChannel Count: {}\tChannel Size: {}\tChannel Order: {}}}",
        isSRGB, channelCount, channelSize, (uint8_t)dataType, (uint8_t)channelOrder);
}

} // namespace graphics