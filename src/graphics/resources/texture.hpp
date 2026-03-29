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

    /// @brief Get Vulkan descriptor for this image. Does not include a sampler.
    /// @return Vulkan descriptor image info
    VkDescriptorImageInfo GetDescriptorInfo() const
    {
        VkDescriptorImageInfo imageInfo = image.GetDescriptorInfo();
        imageInfo.sampler = sampler.Get();
        return imageInfo;
    }
private:
    internal::Device &device;
    Image image;
    Sampler sampler;
};

} // namespace graphics