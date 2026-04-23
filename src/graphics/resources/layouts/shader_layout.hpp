#pragma once
#include "var_layout.hpp"
#include "graphics/api/texture_properties.hpp"

#include <cstdint>
#include <vector>
#include <string_view>
#include <string>
#include <unordered_map>

#include "graphics/standards.hpp"

class SpvReflectBlockVariable;
class SpvReflectInterfaceVariable;
class SpvReflectDescriptorBinding;

namespace graphics
{

class SlangReflect;
class SpirvReflect;

struct ShaderParameter
{
    struct Metadata
    {
        bool useMin = false;
        float min = 0;
        bool useMax = false;
        float max = 1;
        bool isColor = false;
    };
    std::string name;
    uint32_t offset;
    TypeDescription type;

    std::string ToString() const;
};

class BufferLayout
{
public:
    BufferLayout() = default;

    /// @brief Get the size of the layout in bytes.
    const uint32_t GetSize() const { return totalSize; }
    /// @brief Get a list of the shader parameters 
    const std::vector<ShaderParameter> &GetParameters() const { return parameters; }
    /// @brief Get a pointer to a single shader parameter
    /// @return Pointer to the shader parameter, nullptr otherwise
    const ShaderParameter *GetParameter(const std::string &name) const
    { 
        auto it = parameterIndex.find(name);
        if(it != parameterIndex.end())
            return &parameters[it->second]; 
        else
            return nullptr;
    }

    std::string ToString() const;
    
private:
    std::vector<ShaderParameter> parameters{};
    std::unordered_map<std::string, uint32_t> parameterIndex{};
    uint32_t totalSize;

    friend class SlangReflect;
    friend class SpirvReflect;
};

class TextureLayout
{
public:

private:
    TextureShape shape{};
    // TODO: Handle texture arrays

    friend class SlangReflect;
    friend class SpirvReflect;
};

// Layout of a shader. Includes all bindings, their type, buffer layouts, and storage properties
class ShaderLayout
{
public:
    struct BindingType
    {
        enum _BindingType : uint8_t
        {
            UniformBuffer,
            StorageBuffer,
            // DynamicUniformBuffer, // TODO: Determine at pipeline creation
            // DynamicStorageBuffer,
            
            SampledImage,
            CombinedImageSampler,
            StorageImage,
            Sampler,
            UniformTexelBuffer,
            StorageTexelBuffer,
            InputAttachment, // Maybe remove

            AccelerationStructure,
            InlineUniformBlock, // Maybe remove

            Invalid = 255
        } type = Invalid;

        inline bool IsBuffer() const noexcept
        {
            return type == UniformBuffer || type == StorageBuffer;
        }
        inline bool IsImage() const noexcept
        {
            return type == SampledImage || type == CombinedImageSampler || type == StorageImage || type == InputAttachment;
        }
        inline bool IsTexelBuffer() const noexcept
        {
            return type == UniformTexelBuffer || type == StorageTexelBuffer;
        }
        inline bool IsSampler() const noexcept
        {
            return type == SampledImage || type == Sampler;
        }

        BindingType() = default;
        BindingType(_BindingType t) : type(t) {};

        bool operator==(BindingType other) const { return type == other.type; }
        bool operator==(_BindingType other) const { return type == other; }
        _BindingType operator()() const { return type; }
        operator _BindingType() { return type; }

        constexpr std::string ToString() const;
    };

    struct BindingInfo
    {
        std::string name;
        uint32_t set = ~0u;
        uint32_t binding = ~0u;
        BindingType type;
        uint32_t bufferIndex = ~0u; // Index in bufferLayouts list
        uint32_t textureIndex = ~0u; // Index in textureLayouts list
        uint32_t count = 1;
        uint32_t stageFlags = 0;
        // uint32_t minBindingSize = 0; // Maybe include for validation
        // NOTE: May want to store image dimensionality for validation
        // TODO: look into descriptor binding flags (VK_EXT_descriptor_indexing)
        bool operator==(const BindingInfo& other) const
        {
            return set == other.set && binding == other.binding && type == other.type;
        }
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

        bool operator==(const DescriptorSetInfo& other) const
        {
            return id == other.id && bindings == other.bindings;
        }
    };

    ShaderLayout() = default;
    ShaderLayout(std::vector<uint32_t> spirv);

    [[nodiscard]] const BufferLayout *GetCameraLayout() const noexcept
    { 
        if(cameraIndex != ~0u)
            return &bufferLayouts[cameraIndex]; 
        return nullptr;
    }
    [[nodiscard]] const BufferLayout *GetWorldLayout() const noexcept
    { 
        if(worldLayout != ~0u)
            return &bufferLayouts[worldLayout]; 
        return nullptr;
    }
    [[nodiscard]] const BufferLayout *GetMaterialLayout() const noexcept
    { 
        if(materialIndex != ~0u)
            return &bufferLayouts[materialIndex]; 
        return nullptr;
    }
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

    const std::vector<BufferLayout>& GetBufferLayouts() const { return bufferLayouts; }
    const std::vector<TextureLayout>& GetTextureLayouts() const { return textureLayouts; }

    std::string ToString() const;
private:

    std::vector<PushConstantRange> pushConstantRanges{};
    std::vector<DescriptorSetInfo> descriptorSets{};
    std::vector<BufferLayout> bufferLayouts{};
    std::vector<TextureLayout> textureLayouts{};
    uint32_t cameraIndex = ~0u; // Should be set 0
    uint32_t worldLayout = ~0u; // Should be set 1
    uint32_t materialIndex = ~0u; // Should be set 2

    void generateBufferInfo(const SpvReflectDescriptorBinding *binding, BindingInfo* info);

    friend class SlangReflect;
    friend class SpirvReflect;
};

inline bool operator==(ShaderLayout::BindingType::_BindingType lhs, const ShaderLayout::BindingType& rhs)
{
    return lhs == rhs.type;
}
} // namespace graphics