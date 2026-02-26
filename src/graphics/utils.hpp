// Header only utilities for graphics module
#pragma once

#include <cstdint>
#include "graphics/resources/shader_layout.hpp"

namespace graphics
{

constexpr uint32_t AlignedSize(uint32_t size, uint32_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

class AlignmentHelper
{
public:
    enum class LayoutType
    {
        Std140 = 0,
        Std430 = 1
    };

    static constexpr uint32_t GetAlignment(ShaderParameter param, LayoutType layout)
    {

    }

    static constexpr uint32_t GetAlignedSize(ShaderParameter param, LayoutType layout)
    {

    }

    static constexpr uint32_t GetTypeAlignment(DataType type, LayoutType layout)
    {
        switch(layout)
        {
            // case LayoutType::Std140: return GetAlignedSize(type);
            // case LayoutType::Std430: return GetTypeAlignmentStd430(type);
        }
    }
private:
    static constexpr uint32_t GetAlignmentStd140(DataType type)
    {
        switch(type)
        {
            case UInt8:
            case Int8:
            case UInt16:
            case Int16:
            case Float16:
            case UInt32:
            case Int32:
            case Float32: return 4;
            case UInt64:
            case Int64:
            case Float64: return 8;

            case Vec2_16: return 4;
            case Vec2_32: return 8;
            case Vec2_64: return 16;

            case Vec3_16: return 8;
            case Vec3_32: return 16;
            case Vec3_64: return 32;

            case Vec4_16: return 8;
            case Vec4_32: return 16;
            case Vec4_64: return 32;

            case Mat2x2_16: return 8;
            case Mat2x2_32: return 16;
            case Mat2x2_64: return 32;

            case Mat3x3_16: return 16;
            case Mat3x3_32: return 32;
            case Mat3x3_64: return 64;

            case Mat4x4_16: return 16;
            case Mat4x4_32: return 32;
            case Mat4x4_64: return 64;

            case Invalid:
            default:
                return 0;
        }
    }

    static constexpr uint32_t GetMemberStrideStd140(DataType type)
    {
        switch(type)
        {
            case UInt8:
            case Int8:
            case UInt16:
            case Int16:
            case Float16:
            case UInt32:
            case Int32:
            case Float32: return 4;
            case UInt64:
            case Int64:
            case Float64: return 8;

            case Vec2_16: return 4;
            case Vec2_32: return 8;
            case Vec2_64: return 16;

            case Vec3_16: return 8;
            case Vec3_32: return 16;
            case Vec3_64: return 32;

            case Vec4_16: return 8;
            case Vec4_32: return 16;
            case Vec4_64: return 32;

            case Mat2x2_16: return 8;
            case Mat2x2_32: return 16;
            case Mat2x2_64: return 32;

            case Mat3x3_16: return 24;
            case Mat3x3_32: return 48;
            case Mat3x3_64: return 96;
            
            case Mat4x4_16: return 32;
            case Mat4x4_32: return 64;
            case Mat4x4_64: return 128;

            case Invalid:
            default:
                return 0;
        }
    }

    static constexpr uint32_t GetArrayStrideStd140(DataType type)
    {
        switch(type)
        {
            case UInt8:
            case Int8:
            case UInt16:
            case Int16:
            case Float16:
            case UInt32:
            case Int32:
            case Float32: return 4;
            case UInt64:
            case Int64:
            case Float64:
            case Vec2: return 8;
            case Vec3:
            case Vec4:
            case Mat2x2: return 16;
            case Mat3x3: return 48;
            case Mat4x4: return 64;
            case Invalid:
            default:
                return 0;
        }
    }

    static constexpr uint32_t GetMemberAlignmentStd430(DataType type)
    {
        switch(type)
        {
            case UInt8:
            case Int8:
            case UInt16:
            case Int16:
            case Float16:
            case UInt32:
            case Int32:
            case Float32: return 4;
            case UInt64:
            case Int64:
            case Float64:
            case Vec2:
            case Mat2x2: return 8;
            case Mat3x3:
            case Vec3:
            case Vec4:
            case Mat4x4: return 16;
            case Invalid:
            default:
                return 0;
        }
    }

    static constexpr uint32_t GetArrayAlignmentStd430(DataType type)
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
            case Float64:
            case Vec2:
            case Mat2x2: return 8;
            case Mat3x3:
            case Vec3: return 12;
            case Vec4:
            case Mat4x4: return 16;
            case Invalid:
            default:
                return 0;
        }
    }

    static constexpr uint32_t GetMemberSizeStd430(DataType type)
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
            case Float64:
            case Vec2: return 8;
            case Vec3:
            case Vec4:
            case Mat2x2: return 16;
            case Mat3x3: return 48;
            case Mat4x4: return 64;
            case Invalid:
            default:
                return 0;
        }
    }

    static constexpr uint32_t GetArraySizeStd430(DataType type)
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
            case Float64:
            case Vec2: return 8;
            case Vec3: return 12;
            case Vec4:
            case Mat2x2: return 16;
            case Mat3x3: return 36;
            case Mat4x4: return 64;
            case Invalid:
            default:
                return 0;
        }
    }
};

} // namespace graphics