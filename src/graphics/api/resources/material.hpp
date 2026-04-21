#pragma once
#include "shader.hpp"
#include "utils/packing.hpp"
#include "graphics/api/handles.hpp"

namespace graphics
{

class TextureData;
class Material
{
public:
    struct TextureBinding
    {
        uint32_t binding;
        TextureHandle handle;
    };

public:
    MaterialHandle graphicsHandle;
    const Shader* shader;
    std::string name;

    Material(const Shader *shader);

    void Update();

    const uint32_t GetBufferSize() const { return data.size(); }
    const std::vector<std::byte> &GetData() const { return data; }
    const std::vector<TextureBinding> &GetTextureBindings() const { return textureBindings; }
    
    inline void SetInt(const std::string &name, int8_t val) { return setIntegral<int8_t>(name, val); }
    inline void SetInt(const std::string &name, int16_t val) { return setIntegral<int16_t>(name, val); }
    inline void SetInt(const std::string &name, int32_t val) { return setIntegral<int32_t>(name, val); }
    inline void SetInt(const std::string &name, int64_t val) { return setIntegral<int64_t>(name, val); }

    inline void SetInt(const std::string &name, uint8_t val) { return setIntegral<uint8_t>(name, val); }
    inline void SetInt(const std::string &name, uint16_t val) { return setIntegral<uint16_t>(name, val); }
    inline void SetInt(const std::string &name, uint32_t val) { return setIntegral<uint32_t>(name, val); }
    inline void SetInt(const std::string &name, uint64_t val) { return setIntegral<uint64_t>(name, val); }

    inline void SetBool(const std::string &name, bool val) { return setIntegral<bool>(name, val); }

    // void SetFloat(std::string_view name, std::float16_t val); // TODO: Deal with 16 bit floats
    inline void SetFloat(const std::string &name, float val) { return setPrimitive<float>(name, val); }
    inline void SetFloat(const std::string &name, double val) { return setPrimitive<double>(name, val); }
    
    inline void SetVector(const std::string &name, glm::i8vec2 val) { return setPrimitive<glm::i8vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u8vec2 val) { return setPrimitive<glm::u8vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::i16vec2 val) { return setPrimitive<glm::i16vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u16vec2 val) { return setPrimitive<glm::u16vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::i32vec2 val) { return setPrimitive<glm::i32vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u32vec2 val) { return setPrimitive<glm::u32vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::i64vec2 val) { return setPrimitive<glm::i64vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u64vec2 val) { return setPrimitive<glm::u64vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::vec2 val) { return setPrimitive<glm::vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::dvec2 val) { return setPrimitive<glm::dvec2>(name, val); }

    inline void SetVector(const std::string &name, glm::i8vec3 val) { return setPrimitive<glm::i8vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u8vec3 val) { return setPrimitive<glm::u8vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::i16vec3 val) { return setPrimitive<glm::i16vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u16vec3 val) { return setPrimitive<glm::u16vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::i32vec3 val) { return setPrimitive<glm::i32vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u32vec3 val) { return setPrimitive<glm::u32vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::i64vec3 val) { return setPrimitive<glm::i64vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u64vec3 val) { return setPrimitive<glm::u64vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::vec3 val) { return setPrimitive<glm::vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::dvec3 val) { return setPrimitive<glm::dvec3>(name, val); }

    inline void SetVector(const std::string &name, glm::i8vec4 val) { return setPrimitive<glm::i8vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u8vec4 val) { return setPrimitive<glm::u8vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::i16vec4 val) { return setPrimitive<glm::i16vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u16vec4 val) { return setPrimitive<glm::u16vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::i32vec4 val) { return setPrimitive<glm::i32vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u32vec4 val) { return setPrimitive<glm::u32vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::i64vec4 val) { return setPrimitive<glm::i64vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u64vec4 val) { return setPrimitive<glm::u64vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::vec4 val) { return setPrimitive<glm::vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::dvec4 val) { return setPrimitive<glm::dvec4>(name, val); }

    inline void SetColor(const std::string &name, Color val) { return setPrimitive<glm::vec3>(name, (glm::vec3)val); }

    void SetMat2x2(std::string_view name, glm::mat2x2 val);
    void SetMat3x3(std::string_view name, glm::mat3x3 val);
    void SetMat4x4(std::string_view name, glm::mat4x4 val);

    void SetTexture(const std::string &name, TextureHandle handle);
    void SetTexture(const std::string &name, const TextureData& texture);
private:
    struct DataIndex
    {
        uint32_t index; // Index in data vector
        TypeDescription type; // Holds size, type, etc.
    };

    // const Shader* shader;
    const ShaderLayout* materialLayout = nullptr;
    std::vector<std::byte> data{};
    std::unordered_map<std::string, DataIndex> dataMap{};
    std::vector<TextureBinding> textureBindings{};
    std::unordered_map<std::string, uint32_t> textureIndex{};

    

    bool validateLayout() const noexcept;
    const ShaderParameter* getShaderParameter(const std::string& paramName, const BufferLayout* materialInfoLayout);

    template<typename T>
    void setIntegral(const std::string& paramName, T value)
    {
        static_assert(
            std::is_integral_v<T> || "Material::setInt requires int or bool type"
        );
        
        if(!validateLayout()) 
            return;
        const BufferLayout* materialInfoLayout = materialLayout->GetMaterialLayout();

        const ShaderParameter *param = getShaderParameter(paramName, materialInfoLayout);
        if(param == nullptr)
            return;
        const TypeDescription& typeInfo = param->type;
        if(typeInfo.type != DataType::Bool &&
            typeInfo.type != DataType::SInt &&
            typeInfo.type != DataType::UInt)
        {
            Console::warnf("Cannot set value of integral type to field {} of type {}", paramName, ToString(typeInfo.type));
        }
        DataIndex index = dataMap.at(paramName);
        std::byte* dest = data.data() + index.index;


        if constexpr (std::is_signed_v<T>)
        {
            switch (typeInfo.GetSize())
            {
                case 1: *reinterpret_cast<int8_t*>(dest) = static_cast<int8_t>(value); break;
                case 2: *reinterpret_cast<int16_t*>(dest) = static_cast<int16_t>(value); break;
                case 4: *reinterpret_cast<int32_t*>(dest) = static_cast<int32_t>(value); break;
                case 8: *reinterpret_cast<int64_t*>(dest) = static_cast<int64_t>(value); break;
                default:
                    Console::warnf("Unsupported signed integer storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else
        {
            switch (typeInfo.GetSize())
            {
                case 1: *reinterpret_cast<uint8_t*>(dest) = static_cast<uint8_t>(value); break;
                case 2: *reinterpret_cast<uint16_t*>(dest) = static_cast<uint16_t>(value); break;
                case 4: *reinterpret_cast<uint32_t*>(dest) = static_cast<uint32_t>(value); break;
                case 8: *reinterpret_cast<uint64_t*>(dest) = static_cast<uint64_t>(value); break;
                default:
                    Console::warnf("Unsupported unsigned integer storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
    }

    template<typename T>
    void setPrimitive(const std::string& paramName, T value)
    {
        static_assert(
            std::is_floating_point_v<T> ||

            std::is_same_v<glm::vec2, T> || std::is_same_v<glm::dvec2, T> || 
            std::is_same_v<glm::i8vec2, T> || std::is_same_v<glm::i16vec2, T> || std::is_same_v<glm::i32vec2, T> || std::is_same_v<glm::i64vec2, T> ||
            std::is_same_v<glm::u8vec2, T> || std::is_same_v<glm::u16vec2, T> || std::is_same_v<glm::u32vec2, T> || std::is_same_v<glm::u64vec2, T> ||

            std::is_same_v<glm::vec3, T> || std::is_same_v<glm::dvec3, T> || 
            std::is_same_v<glm::i8vec3, T> || std::is_same_v<glm::i16vec3, T> || std::is_same_v<glm::i32vec3, T> || std::is_same_v<glm::i64vec3, T> ||
            std::is_same_v<glm::u8vec3, T> || std::is_same_v<glm::u16vec3, T> || std::is_same_v<glm::u32vec3, T> || std::is_same_v<glm::u64vec3, T> ||

            std::is_same_v<glm::vec4, T> || std::is_same_v<glm::dvec4, T> || 
            std::is_same_v<glm::i8vec4, T> || std::is_same_v<glm::i16vec4, T> || std::is_same_v<glm::i32vec4, T> || std::is_same_v<glm::i64vec4, T> ||
            std::is_same_v<glm::u8vec4, T> || std::is_same_v<glm::u16vec4, T> || std::is_same_v<glm::u32vec4, T> || std::is_same_v<glm::u64vec4, T>
        );

        if(!validateLayout()) 
            return;
        const BufferLayout* materialInfoLayout = materialLayout->GetMaterialLayout();

        const ShaderParameter *param = getShaderParameter(paramName, materialInfoLayout);
        if(param == nullptr)
            return;
        const TypeDescription& typeInfo = param->type;
        DataIndex index = dataMap.at(paramName);
        std::byte* dest = data.data() + index.index;
        
        if constexpr (std::is_floating_point_v<T>)
        {
            switch (typeInfo.GetSize())
            {
                // case 2: // TODO: Handle float to half conversion
                // {
                //     const float f = static_cast<float>(value);
                //     *reinterpret_cast<uint16_t*>(dest) = floatToHalf(f);
                //     break;
                // }
                case 4: *reinterpret_cast<float*>(dest) = static_cast<float>(value); break;
                case 8: *reinterpret_cast<double*>(dest) = static_cast<double>(value); break;
                default:
                    Console::warnf("Unsupported floating-point storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        // Vec2
        else if constexpr (std::is_same_v<glm::vec2, T> || std::is_same_v<glm::dvec2, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 4: *reinterpret_cast<uint32_t*>(dest) = glm::packHalf2x16(static_cast<glm::vec2>(value)); break;
                case 8: *reinterpret_cast<glm::vec2*>(dest) = static_cast<glm::vec2>(value); break;
                case 16: *reinterpret_cast<glm::dvec2*>(dest) = static_cast<glm::dvec2>(value); break;
                default:
                    Console::warnf("Unsupported vec2 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else if constexpr (std::is_same_v<glm::i8vec2, T> || std::is_same_v<glm::i16vec2, T> || std::is_same_v<glm::i32vec2, T> || std::is_same_v<glm::i64vec2, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 2: *reinterpret_cast<glm::u8vec2*>(dest) = static_cast<glm::u8vec2>(value); break;
                case 4: *reinterpret_cast<glm::u16vec2*>(dest) = static_cast<glm::u16vec2>(value); break;
                case 8: *reinterpret_cast<glm::u32vec2*>(dest) = static_cast<glm::u32vec2>(value); break;
                case 16: *reinterpret_cast<glm::u64vec2*>(dest) = static_cast<glm::u64vec2>(value); break;
                default:
                    Console::warnf("Unsupported uvec2 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else if constexpr (std::is_same_v<glm::u8vec2, T> || std::is_same_v<glm::u16vec2, T> || std::is_same_v<glm::u32vec2, T> || std::is_same_v<glm::u64vec2, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 2: *reinterpret_cast<glm::i8vec2*>(dest) = static_cast<glm::i8vec2>(value); break;
                case 4: *reinterpret_cast<glm::i16vec2*>(dest) = static_cast<glm::i16vec2>(value); break;
                case 8: *reinterpret_cast<glm::i32vec2*>(dest) = static_cast<glm::i32vec2>(value); break;
                case 16: *reinterpret_cast<glm::i64vec2*>(dest) = static_cast<glm::i64vec2>(value); break;
                default:
                    Console::warnf("Unsupported ivec2 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        // Vec3
        else if constexpr (std::is_same_v<glm::vec3, T> || std::is_same_v<glm::dvec3, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 6:
                {
                    uint16_t packedVec3[3] = {
                        PackFloatToHalf(value.x),
                        PackFloatToHalf(value.y),
                        PackFloatToHalf(value.z)
                    };
                    memcpy(dest, packedVec3, sizeof(packedVec3));
                    break;
                }
                case 12: *reinterpret_cast<glm::vec3*>(dest) = static_cast<glm::vec3>(value); break;
                case 24: *reinterpret_cast<glm::dvec3*>(dest) = static_cast<glm::dvec3>(value); break;
                default:
                    Console::warnf("Unsupported vec3 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else if constexpr (std::is_same_v<glm::i8vec3, T> || std::is_same_v<glm::i16vec3, T> || std::is_same_v<glm::i32vec3, T> || std::is_same_v<glm::i64vec3, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 3: *reinterpret_cast<glm::u8vec3*>(dest) = static_cast<glm::u8vec3>(value); break;
                case 6: *reinterpret_cast<glm::u16vec3*>(dest) = static_cast<glm::u16vec3>(value); break;
                case 12: *reinterpret_cast<glm::u32vec3*>(dest) = static_cast<glm::u32vec3>(value); break;
                case 24: *reinterpret_cast<glm::u64vec3*>(dest) = static_cast<glm::u64vec3>(value); break;
                default:
                    Console::warnf("Unsupported uvec3 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else if constexpr (std::is_same_v<glm::u8vec3, T> || std::is_same_v<glm::u16vec3, T> || std::is_same_v<glm::u32vec3, T> || std::is_same_v<glm::u64vec3, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 3: *reinterpret_cast<glm::i8vec3*>(dest) = static_cast<glm::i8vec3>(value); break;
                case 6: *reinterpret_cast<glm::i16vec3*>(dest) = static_cast<glm::i16vec3>(value); break;
                case 12: *reinterpret_cast<glm::i32vec3*>(dest) = static_cast<glm::i32vec3>(value); break;
                case 24: *reinterpret_cast<glm::i64vec3*>(dest) = static_cast<glm::i64vec3>(value); break;
                default:
                    Console::warnf("Unsupported ivec3 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        // Vec4
        else if constexpr (std::is_same_v<glm::vec4, T> || std::is_same_v<glm::dvec4, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 8:
                {
                    uint16_t packedVec4[4] = {
                        PackFloatToHalf(value.x),
                        PackFloatToHalf(value.y),
                        PackFloatToHalf(value.z),
                        PackFloatToHalf(value.w)
                    };
                    memcpy(dest, packedVec4, sizeof(packedVec4));
                    break;
                }
                case 16: *reinterpret_cast<glm::vec4*>(dest) = static_cast<glm::vec4>(value); break;
                case 32: *reinterpret_cast<glm::dvec4*>(dest) = static_cast<glm::dvec4>(value); break;
                default:
                    Console::warnf("Unsupported vec4 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else if constexpr (std::is_same_v<glm::i8vec4, T> || std::is_same_v<glm::i16vec4, T> || std::is_same_v<glm::i32vec4, T> || std::is_same_v<glm::i64vec4, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 4: *reinterpret_cast<glm::u8vec4*>(dest) = static_cast<glm::u8vec4>(value); break;
                case 8: *reinterpret_cast<glm::u16vec4*>(dest) = static_cast<glm::u16vec4>(value); break;
                case 16: *reinterpret_cast<glm::u32vec4*>(dest) = static_cast<glm::u32vec4>(value); break;
                case 32: *reinterpret_cast<glm::u64vec4*>(dest) = static_cast<glm::u64vec4>(value); break;
                default:
                    Console::warnf("Unsupported uvec4 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else if constexpr (std::is_same_v<glm::u8vec4, T> || std::is_same_v<glm::u16vec4, T> || std::is_same_v<glm::u32vec4, T> || std::is_same_v<glm::u64vec4, T>)
        {
            switch(typeInfo.GetSize())
            {
                case 4: *reinterpret_cast<glm::i8vec4*>(dest) = static_cast<glm::i8vec4>(value); break;
                case 8: *reinterpret_cast<glm::i16vec4*>(dest) = static_cast<glm::i16vec4>(value); break;
                case 16: *reinterpret_cast<glm::i32vec4*>(dest) = static_cast<glm::i32vec4>(value); break;
                case 32: *reinterpret_cast<glm::i64vec4*>(dest) = static_cast<glm::i64vec4>(value); break;
                default:
                    Console::warnf("Unsupported ivec4 storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }

        // Console::logf("Set parameter {} at offset {}", name, index, "Material");
    }

    template<typename T>
    void setTensor(const std::string &name, T value)
    {
        static_assert(
            std::is_same_v<glm::vec2, T> || std::is_same_v<glm::vec3, T> || std::is_same_v<glm::vec4, T> ||
            std::is_same_v<glm::ivec2, T> || std::is_same_v<glm::ivec3, T> || std::is_same_v<glm::ivec4, T> ||
            std::is_same_v<glm::uvec2, T> || std::is_same_v<glm::uvec3, T> || std::is_same_v<glm::uvec4, T>
        );
    }
    // glm::packHalf2x16()
};

} // namespace graphics