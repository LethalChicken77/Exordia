#include "texture.hpp"
#include "graphics/graphics_data.hpp"

namespace graphics
{

ImageProperties GetImageProperties(const TextureData &texture)
{
    const TextureConfig& config = texture.properties;
    ImageProperties props{};
    if(imageFormatToVkFormat.contains(config.format))
        props.format = imageFormatToVkFormat.at(config.format);
    else
    {
        Console::error("Unsupported format", "Texture");
        return props;
    }
    switch(config.type) // TODO: Make more of these options configurable
    {
    default:
    case TextureType::Default:
        props.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        props.imageType = VK_IMAGE_TYPE_2D;
        props.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
        props.sampleCount = VK_SAMPLE_COUNT_1_BIT;
        props.mipLevels = 1;
        props.arrayLayers = 1;
        break;
    case TextureType::NormalMap:
        props.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        props.imageType = VK_IMAGE_TYPE_2D;
        props.imageViewType = VK_IMAGE_VIEW_TYPE_2D;
        props.sampleCount = VK_SAMPLE_COUNT_1_BIT;
        props.mipLevels = 1;
        props.arrayLayers = 1;
        break;
    case TextureType::CubeMap:
        props.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        props.imageType = VK_IMAGE_TYPE_2D;
        props.imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
        props.sampleCount = VK_SAMPLE_COUNT_1_BIT;
        props.mipLevels = 1;
        props.arrayLayers = 1;
    }
    return props;
}

Texture::Texture(internal::Device &_device, const TextureData &texture)
    : device(_device),
    image(device, texture, GetImageProperties(texture)),
    sampler(device, texture.properties)
{

}

Texture::Texture(const TextureData &texture)
    : device(graphicsData->GetBackend().GetDevice()),
    image(device, texture, GetImageProperties(texture)),
    sampler(device, texture.properties)
{

}

} // namespace graphics