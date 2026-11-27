#pragma once
#include "image.hpp"
#include "sampler.hpp"

namespace graphics
{

/// @brief Conainer for an image and a sampler.
/// @note The sampler is required, for storage textures just use an image.
class TextureResource
{
public:
    TextureResource(internal::Device &_device, Texture data);
    TextureResource(Texture data);

    /// @brief Get Vulkan descriptor for this image. Does not include a sampler.
    /// @return Vulkan descriptor image info
    VkDescriptorImageInfo GetDescriptorInfo() const
    {
        VkDescriptorImageInfo imageInfo = image.GetDescriptorInfo();
        imageInfo.sampler = sampler.Get();
        return imageInfo;
    }

    const Image& GetImage() const { return image; }
    const Sampler& GetSampler() const { return sampler; }
private:
    internal::Device &device;
    Image image;
    Sampler sampler;
};

} // namespace graphics