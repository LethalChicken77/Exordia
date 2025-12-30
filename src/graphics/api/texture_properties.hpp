#pragma once

namespace graphics
{
enum class TextureType
{
    DEFAULT = 0, // Color, linear channels
    NORMAL_MAP = 1, // Normal maps, perform remapping beforehand for performance
    CUBE_MAP = 2, // Cube maps, divided into 6 textures in the graphics module
    // LIGHT_MAP, SPRITE, HDRI, etc.
};

enum class TextureWrapMode
{
    REPEAT = 0,
    MIRRORED_REPEAT = 1,
    CLAMP_TO_EDGE = 2,
    CLAMP_TO_BORDER = 3,
    MIRROR_CLAMP_TO_EDGE = 4
};

enum class TextureFilterMode
{
    NEAREST = 0,
    BILINEAR = 1,
    TRILINEAR = 2,
    ANISOTROPIC = 3
};

enum class MipmapMode
{
    NONE = 0,
    NEAREST = 1,
    LINEAR = 2
};

enum class SwizzleChannel
{
    RED = 0,
    GREEN = 1,
    BLUE = 2,
    ALPHA = 3,
    ZERO = 4,
    ONE = 5
};

struct Swizzle
{
    SwizzleChannel r = SwizzleChannel::RED;
    SwizzleChannel g = SwizzleChannel::GREEN;
    SwizzleChannel b = SwizzleChannel::BLUE;
    SwizzleChannel a = SwizzleChannel::ALPHA;
};

struct TextureProperties
{
    // Preprocessing properties
    TextureType type = TextureType::DEFAULT; // Perform preprocessing based on type
    Swizzle swizzle; // Swizzle channels
    bool generateMipmaps = false; // Change to true when mipmaps are implemented

    // General properties
    TextureWrapMode wrapU = TextureWrapMode::REPEAT;
    TextureWrapMode wrapV = TextureWrapMode::REPEAT;
    TextureWrapMode wrapW = TextureWrapMode::REPEAT; // Unused for 2D textures
    TextureFilterMode filterMode = TextureFilterMode::TRILINEAR;
    MipmapMode mipmapMode = MipmapMode::LINEAR;
    bool srgb = true; // Color textures
    bool readWrite = false; // Storage textures, default false for sampled textures
};
} // namespace graphics