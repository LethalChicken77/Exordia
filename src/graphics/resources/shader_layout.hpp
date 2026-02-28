#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#include <string>

namespace graphics
{
// Shader layout info
enum DataType
{
    Invalid = -1,
    
    UInt8 = 0,
    Int8 = 1,
    UInt16 = 2,
    Int16 = 3,
    UInt32 = 4,
    Int32 = 5,
    UInt64 = 6,
    Int64 = 7,

    UNorm_16 = 8,
    SNorm_16 = 9,
    UNorm_32 = 10,
    SNorm_32 = 11,
    UNorm_64 = 12,
    SNorm_64 = 13,
    
    Float16 = 14,
    Float32 = 15,
    Float64 = 16,
    Vec2_16 = 17,
    Vec2_32 = 18,
    Vec2_64 = 19,
    Vec3_16 = 20,
    Vec3_32 = 21,
    Vec3_64 = 22,
    Vec4_16 = 23,
    Vec4_32 = 24,
    Vec4_64 = 25,

    Mat2x2_16 = 26,
    Mat2x2_32 = 27,
    Mat2x2_64 = 28,
    Mat3x3_16 = 29,
    Mat3x3_32 = 30,
    Mat3x3_64 = 31,
    Mat4x4_16 = 32,
    Mat4x4_32 = 33,
    Mat4x4_64 = 34,

    Char = Int8,
    UChar = UInt8,
    Byte = Int8,
    UByte = UInt8,
    Half = Float16,
    Double = Float64,
    Vec2 = Vec2_32,
    Vec3 = Vec3_32,
    Vec4 = Vec4_32,
    Mat2x2 = Mat2x2_32,
    Mat3x3 = Mat3x3_32,
    Mat4x4 = Mat4x4_32,
    Color = Vec4,
    Transform = Mat4x4,
};

/// @brief Get size of a data type in bytes
/// @param type 
/// @return Size in bytes. 0 if invalid.
constexpr size_t GetTypeSize(DataType type)
{
    switch(type)
    {
        case UInt8:
        case Int8: return 1;
        case UInt16:
        case Int16:
        case Float16: return 2;
        case UInt32:
        case Int32:
        case Float32: return 4;
        case UInt64:
        case Int64:
        case Float64: return 8;

        case Vec2_16: return 4;
        case Vec2_32: return 8;
        case Vec2_64: return 16;

        case Vec3_16: return 6;
        case Vec3_32: return 12;
        case Vec3_64: return 24;

        case Vec4_16: return 8;
        case Vec4_32: return 16;
        case Vec4_64: return 32;

        case Mat2x2_16: return 8;
        case Mat2x2_32: return 16;
        case Mat2x2_64: return 32;

        case Mat3x3_16: return 18;
        case Mat3x3_32: return 36;
        case Mat3x3_64: return 72;

        case Mat4x4_16: return 32;
        case Mat4x4_32: return 64;
        case Mat4x4_64: return 128;

        case Invalid:
        default:
            return 0;
    }
}



struct ShaderParameter
{
    uint32_t offset;
    std::string name;
    DataType type;
    uint32_t typeSize;
    uint32_t count;
};

class ShaderLayout
{
public:
    ShaderLayout() = default;
    ShaderLayout(std::vector<uint32_t> spirv, const std::string_view bufferName);

    
    
private:
    std::vector<ShaderParameter> parameters;

    enum class AlignmentType
    {
        Invalid = -1,

        Scal8 = 0,
        Scal16 = 1,
        Scal32 = 2,
        Scal64 = 3,

        Vec2_8 = 4,
        Vec2_16 = 5,
        Vec2_32 = 6,
        Vec2_64 = 7,
        
        Vec3_8 = 8,
        Vec3_16 = 9,
        Vec3_32 = 10,
        Vec3_64 = 11,

        Vec4_8 = 12,
        Vec4_16 = 13,
        Vec4_32 = 14,
        Vec4_64 = 15,

        Mat2x2_8 = 16,
        Mat2x2_16 = 17,
        Mat2x2_32 = 18,
        Mat2x2_64 = 19,

        Mat3x3_8 = 20,
        Mat3x3_16 = 21,
        Mat3x3_32 = 22,
        Mat3x3_64 = 23,

        Mat4x4_8 = 24,
        Mat4x4_16 = 25,
        Mat4x4_32 = 26,
        Mat4x4_64 = 27
    };
};
};