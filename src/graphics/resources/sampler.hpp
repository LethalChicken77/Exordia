#pragma once
#include "graphics/backend/device.hpp"
#include "graphics/api/texture_properties.hpp"

namespace graphics
{

/// @brief Wrapper for a VkSampler
class Sampler
{
public:
    Sampler(internal::Device& device, const TextureConfig &properties);
    ~Sampler();

    const VkSampler &Get() const { return sampler; }
private:
    internal::Device& device;
    VkSampler sampler = VK_NULL_HANDLE;
};

} // namespace graphics