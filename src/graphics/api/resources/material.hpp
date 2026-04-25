#pragma once
#include "shader.hpp"
#include "utils/packing.hpp"
#include "graphics/api/handles.hpp"

namespace graphics
{

template<typename T>
concept IsVec = requires { T::length(); typename T::value_type; };
template<typename T>
concept IsVecFloat = IsVec<T> && std::is_floating_point_v<typename T::value_type>;
template<typename T>
concept IsVecInt = IsVec<T> && std::is_integral_v<typename T::value_type>;

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

    Material(const Shader *shader, const std::string_view name = "New Material");
    ~Material();

    void UpdateLayout(bool updateVals = true);
    void UpdateValues();

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
    inline void SetFloat(const std::string &name, float val) { return setFloat<float>(name, val); }
    inline void SetFloat(const std::string &name, double val) { return setFloat<double>(name, val); }
    
    inline void SetVector(const std::string &name, glm::i8vec2 val) { return setIntegral<glm::i8vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u8vec2 val) { return setIntegral<glm::u8vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::i16vec2 val) { return setIntegral<glm::i16vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u16vec2 val) { return setIntegral<glm::u16vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::i32vec2 val) { return setIntegral<glm::i32vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u32vec2 val) { return setIntegral<glm::u32vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::i64vec2 val) { return setIntegral<glm::i64vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::u64vec2 val) { return setIntegral<glm::u64vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::vec2 val) { return setFloat<glm::vec2>(name, val); }
    inline void SetVector(const std::string &name, glm::dvec2 val) { return setFloat<glm::dvec2>(name, val); }

    inline void SetVector(const std::string &name, glm::i8vec3 val) { return setIntegral<glm::i8vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u8vec3 val) { return setIntegral<glm::u8vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::i16vec3 val) { return setIntegral<glm::i16vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u16vec3 val) { return setIntegral<glm::u16vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::i32vec3 val) { return setIntegral<glm::i32vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u32vec3 val) { return setIntegral<glm::u32vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::i64vec3 val) { return setIntegral<glm::i64vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::u64vec3 val) { return setIntegral<glm::u64vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::vec3 val) { return setFloat<glm::vec3>(name, val); }
    inline void SetVector(const std::string &name, glm::dvec3 val) { return setFloat<glm::dvec3>(name, val); }

    inline void SetVector(const std::string &name, glm::i8vec4 val) { return setIntegral<glm::i8vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u8vec4 val) { return setIntegral<glm::u8vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::i16vec4 val) { return setIntegral<glm::i16vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u16vec4 val) { return setIntegral<glm::u16vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::i32vec4 val) { return setIntegral<glm::i32vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u32vec4 val) { return setIntegral<glm::u32vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::i64vec4 val) { return setIntegral<glm::i64vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::u64vec4 val) { return setIntegral<glm::u64vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::vec4 val) { return setFloat<glm::vec4>(name, val); }
    inline void SetVector(const std::string &name, glm::dvec4 val) { return setFloat<glm::dvec4>(name, val); }

    inline void SetColor(const std::string &name, Color val) { return setFloat<glm::vec3>(name, (glm::vec3)val); }
    inline void SetColor3(const std::string &name, Color val) { return setFloat<glm::vec3>(name, (glm::vec3)val); }
    inline void SetColor4(const std::string &name, Color val) { return setFloat<glm::vec4>(name, (glm::vec4)val); }

    void SetMat2x2(std::string_view name, glm::mat2x2 val);
    void SetMat3x3(std::string_view name, glm::mat3x3 val);
    void SetMat4x4(std::string_view name, glm::mat4x4 val);

    void SetTexture(const std::string &name, TextureHandle handle);
    void SetTexture(const std::string &name, const TextureData& texture);

    /// @brief Draw material properties in ImGui. Expects window to already be in progress.
    void DrawImGui();
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

    /// @brief Write an integral value to the buffer
    /// @tparam T Source type
    /// @param paramName Shader parameter to find
    /// @param value Value to cast into the buffer
    template<typename T>
    void setIntegral(const std::string& paramName, T value)
    {
        static_assert(
            std::is_integral_v<T> || IsVecInt<T>, 
            "Material::setIntegral requires int or bool type"
        );
        
        if(!validateLayout()) 
            return;
        const BufferLayout* materialInfoLayout = materialLayout->GetMaterialLayout();

        const ShaderParameter *param = getShaderParameter(paramName, materialInfoLayout);
        if(param == nullptr)
            return;
        
        setIntegralParam<T>(param, value);

        UpdateValues();
    }

    /// @brief Write a float value to the buffer
    /// @tparam T Source type
    /// @param paramName Shader parameter to find
    /// @param value Value to cast into the buffer
    template<typename T>
    void setFloat(const std::string& paramName, T value)
    {
        static_assert(
            std::is_floating_point_v<T> || IsVecFloat<T>, 
            "Material::setFloat requires floating point type"
        );
        
        if(!validateLayout()) 
            return;
        const BufferLayout* materialInfoLayout = materialLayout->GetMaterialLayout();

        const ShaderParameter *param = getShaderParameter(paramName, materialInfoLayout);
        if(param == nullptr)
            return;
        setFloatParam<T>(param, value);
        UpdateValues();
    }
    // glm::packHalf2x16()
    template<typename T>
    void setIntegralParam(const ShaderParameter* param, T value)
    {
        static_assert(
            std::is_integral_v<T> || IsVecInt<T>, 
            "Material::setIntegralParam requires int or bool type"
        );
        const std::string& paramName = param->name;

        const TypeDescription& typeInfo = param->type;
        if(typeInfo.type != DataType::Bool &&
            typeInfo.type != DataType::SInt &&
            typeInfo.type != DataType::UInt)
        {
            Console::warnf("Cannot set value of integral type to field {} of type {}", paramName, typeInfo.type.ToString());
            return;
        }
        DataIndex index = dataMap.at(paramName);
        std::byte* dest = data.data() + index.index;

        constexpr uint32_t componentCount = []() constexpr
        {
            if constexpr (IsVec<T>)
                return T::length();
            else
                return 1;
        }();

        if(componentCount != typeInfo.componentCount)
        {
            Console::errorf("Type component count does not match template function setIntegral count {}. Param: {}", componentCount, param->ToString());
            return;
        }

        if constexpr(IsVec<T>)
        {
            switch (typeInfo.componentSize)
            {
                case 1:
                    for(uint32_t i = 0; i < componentCount; i++)
                    {
                        reinterpret_cast<uint8_t*>(dest)[i] = static_cast<uint8_t>(value[i]);
                    }
                    break;
                case 2:
                    for(uint32_t i = 0; i < componentCount; i++)
                    {
                        reinterpret_cast<uint16_t*>(dest)[i] = static_cast<uint16_t>(value[i]);
                    }
                    break;
                case 4:
                    for(uint32_t i = 0; i < componentCount; i++)
                    {
                        reinterpret_cast<uint32_t*>(dest)[i] = static_cast<uint32_t>(value[i]);
                    }
                    break;
                case 8:
                    for(uint32_t i = 0; i < componentCount; i++)
                    {
                        reinterpret_cast<uint64_t*>(dest)[i] = static_cast<uint64_t>(value[i]);
                    }
                    break;
                default:
                    Console::warnf("Unsupported integral vector storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else
        {
            switch (typeInfo.componentSize)
            {
                case 1: *reinterpret_cast<uint8_t*>(dest) = static_cast<uint8_t>(value); break;
                case 2: *reinterpret_cast<uint16_t*>(dest) = static_cast<uint16_t>(value); break;
                case 4: *reinterpret_cast<uint32_t*>(dest) = static_cast<uint32_t>(value); break;
                case 8: *reinterpret_cast<uint64_t*>(dest) = static_cast<uint64_t>(value); break;
                default:
                    Console::warnf("Unsupported integer storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
    }

    template<typename T>
    void setFloatParam(const ShaderParameter* param, T value)
    {
        static_assert(
            std::is_floating_point_v<T> || IsVecFloat<T>, 
            "Material::setFloatParam requires floating point type"
        );
        const std::string& paramName = param->name;
        
        const TypeDescription& typeInfo = param->type;
        if(typeInfo.type != DataType::Float)
        {
            Console::warnf("Cannot set value of floating point type to field {} of type {}", paramName, typeInfo.type.ToString());
            return;
        }
        DataIndex index = dataMap.at(paramName);
        std::byte* dest = data.data() + index.index;

        constexpr uint32_t componentCount = []() constexpr
        {
            if constexpr (IsVec<T>)
                return T::length();
            else
                return 1;
        }();

        if(componentCount != typeInfo.componentCount)
        {
            Console::errorf("Type component count does not match template function setFloat count {}. Param: {}", componentCount, param->ToString());
            return;
        }

        if constexpr(IsVec<T>)
        {
            switch (typeInfo.componentSize)
            {
                case 2:
                    for(uint32_t i = 0; i < componentCount; i++)
                    {
                        reinterpret_cast<uint16_t*>(dest)[i] = PackFloatToHalf(value[i]); 
                    }
                    break;
                case 4:
                    for(uint32_t i = 0; i < componentCount; i++)
                    {
                        reinterpret_cast<float*>(dest)[i] = static_cast<float>(value[i]); 
                    }
                    break;
                case 8:
                {
                    for(uint32_t i = 0; i < componentCount; i++)
                    {
                        reinterpret_cast<double*>(dest)[i] = static_cast<double>(value[i]); 
                    }
                    break;
                }
                default:
                    Console::warnf("Unsupported floating point vector storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
        else
        {
            switch (typeInfo.componentSize)
            {
                case 2: *reinterpret_cast<uint16_t*>(dest) = PackFloatToHalf(value); break;
                case 4: *reinterpret_cast<float*>(dest) = static_cast<float>(value); break;
                case 8: *reinterpret_cast<double*>(dest) = static_cast<double>(value); break;
                default:
                    Console::warnf("Unsupported floating point storage size {} for parameter '{}'", typeInfo.GetSize(), name, "Material");
                    return;
            }
        }
    }

    bool drawImGuiParam(const ShaderParameter& param);
};

} // namespace graphics