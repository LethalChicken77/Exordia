#pragma once
#include <cstdint>
#include <vector>
#include <string_view>
#include <string>
#include <unordered_map>

#include "descriptor_standards.hpp"

class SpvReflectBlockVariable;
class SpvReflectDescriptorBinding;
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

    Struct
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
        case DataType::Struct: return "Struct";
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
class BufferLayout
{
public:
    BufferLayout() = default;
    BufferLayout(const SpvReflectBlockVariable* module);

    /// @brief Get the size of the layout in bytes.
    const uint32_t GetSize() const { return totalSize; }
    /// @brief Get a list of the shader parameters 
    const std::vector<ShaderParameter> &GetParameters() const { return parameters; }
    /// @brief Get a pointer to a single shader parameter
    /// @return Pointer to the shader parameter, nullptr otherwise
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


// Layout of a shader. Includes all bindings, their type, buffer layouts, and storage properties
class ShaderLayout
{
public:
    enum BindingType : uint8_t
    {
        UniformBuffer,
        StorageBuffer,
        DynamicUniformBuffer,
        DynamicStorageBuffer,
        
        SampledImage,
        CombinedImageSampler,
        StorageImage,
        Sampler,
        UniformTexelBuffer,
        StorageTexelBuffer,
        InputAttachment,

        AccelerationStructure,
        InlineUniformBlock,

        Invalid = 255
    };

    struct BindingInfo
    {
        std::string name;
        uint32_t set = ~0u;
        uint32_t binding = ~0u;
        BindingType type;
        uint32_t bufferIndex = ~0u; // Index in bufferLayouts list
        uint32_t count = 1;
        uint32_t stageFlags = 0;
        // uint32_t minBindingSize = 0; // Maybe include for validation
        // NOTE: May want to store image dimensionality for validation
        // TODO: look into descriptor binding flags (VK_EXT_descriptor_indexing)
    };

    struct PushConstantRange
    {
        uint32_t stageFlags;
        BufferLayout layout;
    };

    struct DescriptorSetInfo
    {
        uint32_t id = ~0u;
        std::vector<BindingInfo> bindings{};
    };

    ShaderLayout() = default;
    ShaderLayout(std::vector<uint32_t> spirv);

    [[nodiscard]] const BufferLayout *GetCameraLayout() const { return cameraInfo; }
    [[nodiscard]] const BufferLayout *GetGlobalLayout() const { return globalInfo; }
    [[nodiscard]] const BufferLayout *GetMaterialLayout() const { return materialInfo; }
    [[nodiscard]] const std::vector<DescriptorSetInfo> &GetDescriptorSets() const { return descriptorSets; }
    [[nodiscard]] const DescriptorSetInfo *GetMaterialDescriptorSet() const 
    { 
        for(const DescriptorSetInfo &info : descriptorSets)
        {
            if(info.id == MATERIAL_DESCRIPTOR_SET)
            {
                return &info;
            }
            // if(info.bindings[0].name == "materialInfo")
            // {
            //     return &info;
            // }
        }
        return nullptr; 
    }
private:

    std::vector<PushConstantRange> pushConstantRanges{};
    std::vector<DescriptorSetInfo> descriptorSets{};
    std::vector<BufferLayout> bufferLayouts{};
    BufferLayout* cameraInfo = nullptr; // Should be set 0
    BufferLayout* globalInfo = nullptr; // Should be set 1
    BufferLayout* materialInfo = nullptr; // Should be set 2

    void generateBufferInfo(const SpvReflectDescriptorBinding *binding, BindingInfo* info);
};
} // namespace graphics