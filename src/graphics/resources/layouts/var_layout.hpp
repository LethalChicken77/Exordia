#pragma once
#include <cstdint>
#include <string>


namespace graphics
{

class DataType
{
public:
    enum _DataType : uint8_t
    {
        Invalid,
        Float,
        UInt, 
        SInt,
        UNorm,
        SNorm,
        
        Bool,
        Struct
    } type;

    inline bool IsIntegral() const noexcept
    {
        return type == UInt || type == SInt || type == UNorm || type == SNorm || type == Bool;
    }

    inline bool IsFloat() const noexcept
    {
        return type == Float;
    }

    DataType() = default;
    DataType(_DataType t) : type(t) {};

    bool operator==(DataType other) const { return type == other.type; }
    bool operator==(_DataType other) const { return type == other; }
    _DataType operator()() const noexcept { return type; }
    operator _DataType() const noexcept { return type; }
    explicit operator uint32_t() const noexcept { return static_cast<uint32_t>(type); }

    constexpr const char* ToString() const
    {
        switch(type)
        {
            case DataType::Invalid:
                return "Invalid";
            case DataType::UInt:
                return "UInt";
            case DataType::SInt:
                return "SInt";
            case DataType::UNorm:
                return "UNorm";
            case DataType::SNorm:
                return "SNorm";
            case DataType::Float:
                return "Float";
            case DataType::Bool:
                return "Bool";
            case DataType::Struct:
                return "Struct";
        }
        return "How";
    }
};

inline bool operator==(DataType::_DataType lhs, const DataType& rhs)
{
    return lhs == rhs.type;
}
} // namespace graphics

namespace graphics{
struct TypeDescription
{
    DataType type = DataType::Float; // Datatype of the attribute. Used for format selection.
    uint8_t componentSize = 4;
    uint8_t componentCount = 1;
    uint8_t rowCount = 1; // Used for matrix types.
    uint32_t arrayCount = 1;
    bool rowMajor; // Only used for matrices

    /// @brief Get the size of the attribute.
    /// @return Size in bytes.
    inline uint32_t GetSize() const { return componentSize * componentCount * rowCount * arrayCount; }
    /// @brief Get the alignment of the attribute.
    /// @param vecAlignment Determines whether to align vec3 as vec4.
    /// @return Alignment in bytes.
    inline uint32_t GetAlignment(bool vecAlignment) const 
    { 
        if(vecAlignment)
        {
            if(componentCount == 3 && componentSize == 4) return 16; // Align vec3 as vec4
            return componentSize * componentCount; 
        }
        else
        {
            return componentSize;
        }
    }

    uint32_t GetVkFormat() const;

    // Utility methods

    inline bool IsScalar() const { return componentCount == 1; }
    inline bool IsVec2() const { return componentCount == 2; }
    inline bool IsVec3() const { return componentCount == 3; }
    inline bool IsVec4() const { return componentCount == 4; }
    inline bool IsMatrix() const { return rowCount > 1 && componentCount > 1; }
    inline bool IsMat3x3() const { return arrayCount == 3 && componentCount == 3; }
    inline bool IsMat4x4() const { return arrayCount == 4 && componentCount == 4; }
    
    std::string ToString() const;

    bool operator==(const TypeDescription& other) const
    {
        return 
            type == other.type && 
            componentSize == other.componentSize &&
            componentCount == other.componentCount && 
            arrayCount == other.arrayCount;
    }

    struct Hash
    {
        uint32_t operator()(const TypeDescription& a) const
        {
            uint32_t hash = 0;

            auto Combine = [](uint32_t& h, uint32_t v)
            {
                h ^= v + 0x9e3779b9u + (h << 6) + (h >> 2);
            };

            Combine(hash, (uint32_t)a.type);
            Combine(hash, (uint32_t)a.componentSize);
            Combine(hash, (uint32_t)a.componentCount);
            Combine(hash, (uint32_t)a.arrayCount);

            return hash;
        }
    };
};

} // namespace graphics