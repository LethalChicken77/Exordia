#pragma once
#include "primitives/color.hpp"
#include "shared_config.hpp"

namespace graphics
{
enum class TextureType
{
    Default = 0, // Color, linear channels
    NormalMap = 1, // Normal maps, perform remapping beforehand for performance
    CubeMap = 2, // Cube maps, divided into 6 textures in the graphics module
    // LIGHT_MAP, SPRITE, HDRI, etc.
};

enum class TextureWrapMode
{
    Repeat = 0,
    MirroredRepeat = 1,
    ClampToEdge = 2,
    ClampToBorder = 3,
    MirrorClampToEdge = 4
};

enum class TextureFilterMode
{
    Nearest = 0,
    Bilinear = 1,
    Cubic = 2
};

enum class MipmapMode
{
    None = 0, // Used to determine whether to create mipmaps. Sets the sample to nearest.
    Nearest = 1,
    Linear = 2
};

enum class BorderColorMode
{
    FTransparentBlack = 0,
    ITransparentBlack = 1,
    FOpaqueBlack = 2,
    IOpaqueBlack = 3,
    FOpaqueWhite = 4,
    IOpaqueWhite = 5,
    FCustom = 1000287003,
    ICustom = 1000287004
};

enum class SwizzleChannel
{
    Red = 0,
    Green = 1,
    Blue = 2,
    Alpha = 3,
    Zero = 4,
    One = 5
};

struct Swizzle
{
    SwizzleChannel r = SwizzleChannel::Red;
    SwizzleChannel g = SwizzleChannel::Green;
    SwizzleChannel b = SwizzleChannel::Blue;
    SwizzleChannel a = SwizzleChannel::Alpha;
};
// typedef struct VkSamplerCreateInfo {
//     VkStructureType         sType;
//     const void*             pNext;
//     VkSamplerCreateFlags    flags;
//     VkFilter                magFilter;
//     VkFilter                minFilter;
//     VkSamplerMipmapMode     mipmapMode;
//     VkSamplerAddressMode    addressModeU;
//     VkSamplerAddressMode    addressModeV;
//     VkSamplerAddressMode    addressModeW;
//     float                   mipLodBias;
//     VkBool32                anisotropyEnable;
//     float                   maxAnisotropy;
//     VkBool32                compareEnable;
//     VkCompareOp             compareOp;
//     float                   minLod;
//     float                   maxLod;
//     VkBorderColor           borderColor;
//     VkBool32                unnormalizedCoordinates;
// } VkSamplerCreateInfo;
struct TextureConfig
{
    // Preprocessing properties
    TextureType type = TextureType::Default; // Perform preprocessing based on type
    Swizzle swizzle; // Swizzle channels
    bool generateMipmaps = false; // Change to true when mipmaps are implemented

    // General properties
    TextureWrapMode wrapU = TextureWrapMode::Repeat;
    TextureWrapMode wrapV = TextureWrapMode::Repeat;
    TextureWrapMode wrapW = TextureWrapMode::Repeat;

    TextureFilterMode magFilter = TextureFilterMode::Bilinear;
    TextureFilterMode minFilter = TextureFilterMode::Bilinear;

    MipmapMode mipmapMode = MipmapMode::None;
    float mipLodBias = 0;

    BorderColorMode borderColorMode = BorderColorMode::FOpaqueBlack;
    Color borderColor = Color(0.f,0.f,0.f,1.f);

    bool anisotropicFiltering = true;
    float maxAnisotropy = 8.0f;

    bool compareEnable = false; // Used for shadow maps
    CompareOp compareOp = CompareOp::Always; // Use LessEqual for shadow maps

    float minLod = 0.0f;
    float maxLod = 1000.0f;

    bool srgb = true; // Color textures
    bool readWrite = false; // Storage textures, default false for sampled textures
};
} // namespace graphics