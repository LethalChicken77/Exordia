#include "texture_resource.hpp"
#include "graphics/api/texture_properties.hpp"
#include "graphics/graphics_data.hpp"
#include "glm/gtc/integer.hpp"
#include "vulkan/vulkan.hpp"

namespace graphics
{

vk::ComponentSwizzle GetSwizzle(SwizzleChannel channel, SwizzleChannel swizzle) {
    if(channel == swizzle)
        return vk::ComponentSwizzle::eIdentity;

    switch(swizzle) {
        case SwizzleChannel::Red:   return vk::ComponentSwizzle::eR;
        case SwizzleChannel::Green: return vk::ComponentSwizzle::eG;
        case SwizzleChannel::Blue:  return vk::ComponentSwizzle::eB;
        case SwizzleChannel::Alpha: return vk::ComponentSwizzle::eA;
        case SwizzleChannel::Zero:  return vk::ComponentSwizzle::eZero;
        case SwizzleChannel::One:   return vk::ComponentSwizzle::eOne;
    }

    return vk::ComponentSwizzle::eIdentity;
}

ImageProperties GetImageProperties(Texture texture)
{
    const TextureConfig& config = texture->properties;
    ImageProperties props{};
    VkFormat fmt = ImageFormatToVkFormat(config.format);
    if(fmt != VK_FORMAT_UNDEFINED)
        props.format = vk::Format(fmt);
    else
    {
        Console::error("Unsupported format", "Texture");
        return props;
    }
    switch(config.type) // TODO: Make more of these options configurable
    {
    default:
    case TextureType::Default:
        props.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        props.imageType = vk::ImageType::e2D;
        props.imageViewType = vk::ImageViewType::e2D;
        props.sampleCount = vk::SampleCountFlagBits::e1;
        props.arrayLayers = 1;
        break;
    case TextureType::NormalMap:
        props.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        props.imageType = vk::ImageType::e2D;
        props.imageViewType = vk::ImageViewType::e2D;
        props.sampleCount = vk::SampleCountFlagBits::e1;
        props.arrayLayers = 1;
        break;
    case TextureType::CubeMap:
        props.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        props.imageType = vk::ImageType::e2D;
        props.imageViewType = vk::ImageViewType::eCube;
        props.sampleCount = vk::SampleCountFlagBits::e1;
        props.arrayLayers = 1;
    }

    if(texture->properties.generateMipmaps)
    {
        props.mipLevels = glm::floor(glm::log2(glm::max(texture->GetWidth(), texture->GetHeight()))) + 1;
        props.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
    }
    else
    {
        props.mipLevels = 1;
        props.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    }
    props.rSwizzle = GetSwizzle(SwizzleChannel::Red,   texture->properties.swizzle.r);
    props.gSwizzle = GetSwizzle(SwizzleChannel::Green, texture->properties.swizzle.g);
    props.bSwizzle = GetSwizzle(SwizzleChannel::Blue,  texture->properties.swizzle.b);
    props.aSwizzle = GetSwizzle(SwizzleChannel::Alpha, texture->properties.swizzle.a);

    return props;
}

TextureResource::TextureResource(internal::Device &_device, Texture texture)
    : device(_device),
    image(device, texture, GetImageProperties(texture)),
    sampler(device, texture->properties)
{
    Console::debugf("Created texture: {}", texture->name);
}

TextureResource::TextureResource(Texture texture)
    : TextureResource(graphicsData->GetBackend().GetDevice(), texture)
{

}

} // namespace graphics