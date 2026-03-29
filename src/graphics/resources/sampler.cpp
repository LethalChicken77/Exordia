#include "sampler.hpp"
#include "debug.hpp"
#include "console.hpp"
#include <glm/glm.hpp>
#include "graphics/backend/backend_data.hpp" // TODO: Remove in favor of vulkan profiles

namespace graphics
{

vk::Filter getFilterMode(TextureFilterMode filterMode, bool cubicSupported)
{
    switch(filterMode)
    {
        case TextureFilterMode::Nearest:
            return vk::Filter::eNearest;
        case TextureFilterMode::Bilinear:
            return vk::Filter::eLinear;
        case TextureFilterMode::Cubic:
            if(cubicSupported)
                return vk::Filter::eCubicEXT;
            else
            {
                Console::warn("Cubic filter mode not supported, falling back to linear.");
                return vk::Filter::eLinear;
            }
    }
}

constexpr vk::SamplerAddressMode getAddressMode(TextureWrapMode wrapMode)
{
    switch(wrapMode)
    {
        case TextureWrapMode::ClampToBorder:
            return vk::SamplerAddressMode::eClampToBorder;
        case TextureWrapMode::ClampToEdge:
            return vk::SamplerAddressMode::eClampToEdge;
        case TextureWrapMode::MirrorClampToEdge:
            return vk::SamplerAddressMode::eMirrorClampToEdge;
        case TextureWrapMode::MirroredRepeat:
            return vk::SamplerAddressMode::eMirroredRepeat;
        case TextureWrapMode::Repeat:
            return vk::SamplerAddressMode::eRepeat;
    }
}

vk::BorderColor getBorderColor(BorderColorMode mode, bool customSupported)
{
    switch(mode)
    {
        case BorderColorMode::FOpaqueBlack:
            return vk::BorderColor::eFloatOpaqueBlack;
        case BorderColorMode::IOpaqueBlack:
            return vk::BorderColor::eIntOpaqueBlack;
        case BorderColorMode::FTransparentBlack:
            return vk::BorderColor::eFloatTransparentBlack;
        case BorderColorMode::ITransparentBlack:
            return vk::BorderColor::eIntTransparentBlack;
        case BorderColorMode::FOpaqueWhite:
            return vk::BorderColor::eFloatOpaqueWhite;
        case BorderColorMode::IOpaqueWhite:
            return vk::BorderColor::eIntOpaqueWhite;
        case BorderColorMode::FCustom:
            if(customSupported)
                return vk::BorderColor::eFloatCustomEXT;
            else
            {
                Console::warn("Custom border color not supported, falling back to float opaque black.");
                return vk::BorderColor::eFloatOpaqueBlack;
            }
        case BorderColorMode::ICustom:
            if(customSupported)
                return vk::BorderColor::eIntCustomEXT;
            else
            {
                Console::warn("Custom border color not supported, falling back to int opaque black.");
                return vk::BorderColor::eIntOpaqueBlack;
            }
    }
}

constexpr vk::CompareOp getCompareOp(CompareOp op)
{
    switch(op)
    {
    case CompareOp::Never:          return vk::CompareOp::eNever;
    case CompareOp::Less:           return vk::CompareOp::eLess;
    case CompareOp::LessEqual:      return vk::CompareOp::eLessOrEqual;
    case CompareOp::Equal:          return vk::CompareOp::eEqual;
    case CompareOp::GreaterEqual:   return vk::CompareOp::eGreaterOrEqual;
    case CompareOp::Greater:        return vk::CompareOp::eGreater;
    case CompareOp::NotEqual:       return vk::CompareOp::eNotEqual;
    case CompareOp::Always:         return vk::CompareOp::eAlways;
    }
}

constexpr vk::SamplerMipmapMode getMipmapMode(MipmapMode mode)
{
    switch(mode)
    {
    case MipmapMode::None:  
    case MipmapMode::Nearest:   return vk::SamplerMipmapMode::eNearest;
    case MipmapMode::Linear:    return vk::SamplerMipmapMode::eLinear;
    }
}

constexpr bool useBorderColor(TextureWrapMode uMode, TextureWrapMode vMode, TextureWrapMode wMode)
{
    return uMode == TextureWrapMode::ClampToBorder ||
        vMode == TextureWrapMode::ClampToBorder ||
        wMode == TextureWrapMode::ClampToBorder;
}

Sampler::Sampler(internal::Device& _device, const TextureConfig &properties)
    : device(_device)
{
    vk::SamplerCustomBorderColorCreateInfoEXT customBorderColorInfo{};
    customBorderColorInfo.customBorderColor = { properties.borderColor.r, properties.borderColor.g, properties.borderColor.b, properties.borderColor.a };

    
    bool allowCubicFiltering = false; // TODO: Check if cubic extension is enabled
    bool allowCustomBorderColor = false; // TODO: Check if custom border color extension is enabled
    bool allowAnisotropy = internal::features.featureChain.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy;

    const VkPhysicalDeviceLimits &deviceLimits = device.GetPhysicalDevice().GetLimits();
    
    vk::SamplerCreateInfo createInfo{};
    createInfo.flags = vk::SamplerCreateFlags();
    createInfo.minFilter = getFilterMode(properties.minFilter, allowCubicFiltering);
    createInfo.magFilter = getFilterMode(properties.magFilter, allowCubicFiltering);
    createInfo.addressModeU = getAddressMode(properties.wrapU);
    createInfo.addressModeV = getAddressMode(properties.wrapV);
    createInfo.addressModeW = getAddressMode(properties.wrapW);
    createInfo.borderColor = getBorderColor(properties.borderColorMode, allowCustomBorderColor);
    
    if(allowAnisotropy)
    {
        createInfo.anisotropyEnable = properties.anisotropicFiltering;
        createInfo.maxAnisotropy = glm::clamp(properties.maxAnisotropy, 1.0f, deviceLimits.maxSamplerAnisotropy);
    }
    else
    {
        createInfo.anisotropyEnable = VK_FALSE;
        createInfo.maxAnisotropy = 1.0f;
    }

    createInfo.mipmapMode = getMipmapMode(properties.mipmapMode);
    createInfo.mipLodBias = glm::clamp(properties.mipLodBias, -deviceLimits.maxSamplerLodBias, deviceLimits.maxSamplerLodBias);

    createInfo.compareEnable = properties.compareEnable;
    createInfo.compareOp = getCompareOp(properties.compareOp);

    createInfo.minLod = properties.minLod;
    createInfo.maxLod = properties.maxLod;
    if(properties.minLod > properties.maxLod)
    {
        createInfo.minLod = 0.0f;
        createInfo.maxLod = vk::LodClampNone;
        Console::warn("Min LOD cannot be set higher than max LOD.", "Sampler");
    }

    createInfo.unnormalizedCoordinates = false;

    if(allowCustomBorderColor && (properties.borderColorMode == BorderColorMode::FCustom || properties.borderColorMode == BorderColorMode::ICustom))
        createInfo.pNext = &customBorderColorInfo;
    else
        createInfo.pNext = nullptr;
    

    VK_CHECK(vkCreateSampler(device.Get(), reinterpret_cast<VkSamplerCreateInfo*>(&createInfo), nullptr, &sampler), "Failed to create sampler");
}

Sampler::~Sampler()
{
    if(sampler)
        vkDestroySampler(device.Get(), sampler, nullptr);
    sampler = VK_NULL_HANDLE;
}

} // namespace graphics