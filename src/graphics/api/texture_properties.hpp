#pragma once
#include "primitives/color.hpp"
#include "shared_config.hpp"

namespace graphics
{
enum class TextureType : uint8_t
{
    Default = 0, // Color, linear channels
    NormalMap = 1, // Normal maps, perform remapping beforehand for performance
    CubeMap = 2, // Cube maps, divided into 6 textures in the graphics module
    // LIGHT_MAP, SPRITE, HDRI, etc.
};

enum class TextureWrapMode : uint8_t
{
    Repeat = 0,
    MirroredRepeat = 1,
    ClampToEdge = 2,
    ClampToBorder = 3,
    MirrorClampToEdge = 4
};

enum class TextureFilterMode : uint8_t
{
    Nearest = 0,
    Bilinear = 1,
    Cubic = 2
};

enum class MipmapMode : uint8_t
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

enum class SwizzleChannel : uint8_t
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

enum class TextureDataType : uint8_t
{
    UInt,
    SInt,
    UNorm,
    SNorm,
    Float,

    Packed_B10G11R11_UFloat,
    Packed_A2R10G10B10_UNorm,

    D16_UNorm,
    D24_UNorm,
    D32_SFloat,
    D16_UNorm_S8_UInt,
    D24_UNorm_S8_UInt,
    D32_SFloat_S8_UInt,
    S8_UInt,

    Invalid
};

enum class ChannelOrder : uint8_t
{
    RGBA,
    BGRA,
    ABGR
};

/// @brief Defines an image format as a set of properties.
/// @note Treat packed formats as single channel, non-srgb, RGBA, Color images. The channel size should match the packed size.
/// These types can be specified in TextureDataType as needed.
struct ImageFormat
{
    bool isSRGB = false;
    uint8_t channelCount = 4;
    uint8_t channelSize = 1;
    TextureDataType dataType = TextureDataType::UInt;
    ChannelOrder channelOrder = ChannelOrder::RGBA;

    uint32_t PixelSize() const { return channelCount * channelSize; }

    std::string ToString() const;

    bool operator==(const ImageFormat& other) const
    {
        return isSRGB == other.isSRGB
            && channelCount == other.channelCount
            && channelSize == other.channelSize
            && dataType == other.dataType
            && channelOrder == other.channelOrder;
    }
};

struct ImageFormatHash
{
    std::size_t operator()(const ImageFormat& fmt) const
    {
        std::size_t h = 0;
        h |= (uint8_t)fmt.isSRGB; // 1 bit (1) - bool value
        h |= ((uint8_t)fmt.channelCount) << 1; // 3 bits (4) - values up to and including 4
        h |= ((uint8_t)fmt.channelSize) << 4; // 5 bits (9) - values up to and including 16
        h |= ((uint8_t)fmt.dataType) << 9; // 8 bits (17) - 14 values but expandible
        h |= ((uint8_t)fmt.channelOrder) << 17; // 2 bits (19) - 3 values
        return h;
    }
};

struct TextureConfig
{
    // Preprocessing properties
    TextureType type = TextureType::Default; // Perform preprocessing based on type
    bool generateMipmaps = false; // Change to true when mipmaps are implemented
    
    // Texture properties
    ImageFormat format{};
    Swizzle swizzle{};

    // Sampler properties
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