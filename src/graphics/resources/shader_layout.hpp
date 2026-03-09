#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#include <string>
#include <unordered_map>


namespace graphics
{
// Shader layout info
enum class DataType
{
    Invalid = -1,
    
    Float,
    Int,
    UInt,
    Bool, // Maybe alias for int?

    Vec2,
    IVec2,
    UVec2,

    Vec3,
    IVec3,
    UVec3,

    Vec4,
    IVec4,
    UVec4,
};

constexpr std::string GetTypeString(DataType type)
{
    switch(type)
    {
        case DataType::Int: return "Int";
        case DataType::UInt: return "UInt";
        case DataType::Float: return "Float";
        case DataType::Vec2: return "Vec2";
        case DataType::IVec2: return "IVec2";
        case DataType::UVec2: return "UVec2";
        case DataType::Vec3: return "Vec3";
        case DataType::IVec3: return "IVec3";
        case DataType::UVec3: return "UVec3";
        case DataType::Vec4: return "Vec4";
        case DataType::IVec4: return "IVec4";
        case DataType::UVec4: return "UVec4";
        case DataType::Invalid:
        default: return "Invalid";
    }
}

struct ShaderParameter
{
    std::string name;
    uint32_t offset;
    DataType type;
    uint8_t baseSize; // Size of single component (ie. vec3.x) in bytes
    uint8_t size; // Total size of an element in bytes (Stride for arrays, may use padding in that case)
    uint32_t count;
    bool rowMajor; // Only used for matrices
};

extern std::string GetParameterString(ShaderParameter param);

class ShaderLayout
{
public:
    ShaderLayout() = default;
    ShaderLayout(std::vector<uint32_t> spirv, const std::string_view bufferName);

    const uint32_t GetSize() const { return totalSize; }
    const std::vector<ShaderParameter> &GetParameters() const { return parameters; }
    const ShaderParameter *GetParameter(const std::string &name) const 
    { 
        if(parameterIndex.contains(name))
            return parameterIndex.at(name); 
        else
            return nullptr;
    }
    
private:
    std::vector<ShaderParameter> parameters{};
    std::unordered_map<std::string, const ShaderParameter*> parameterIndex{};
    uint32_t set;
    uint32_t binding;
    uint32_t totalSize;
};
} // namespace graphics