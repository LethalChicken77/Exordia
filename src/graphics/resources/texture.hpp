#pragma once
#include "image.hpp"
#include "sampler.hpp"

namespace graphics
{

/// @brief Conainer for an image and a sampler.
/// @note The sampler is required, for storage textures just use an image.
class Texture
{
public:
    Texture(internal::Device &_device, const TextureData &data);
    Texture(const TextureData &data);
private:
    internal::Device &device;
    Image image;
    Sampler sampler;
};

} // namespace graphics