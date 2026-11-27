#pragma once
#include "graphics/api/assets/shader_asset.hpp"
#include "graphics/api/resources/texture_data.hpp"
#include "utils/packing.hpp"
#include "graphics/api/handles.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/type_trait.hpp>

#include "console.hpp"

namespace graphics
{

template<typename T>
concept IsVec = requires { T::length(); typename T::value_type; };
template<typename T>
concept IsVecFloat = IsVec<T> && std::is_floating_point_v<typename T::value_type>;
template<typename T>
concept IsVecInt = IsVec<T> && std::is_integral_v<typename T::value_type>;

class TextureData;
class Material;
class MaterialData
{
public:
    struct TextureBinding
    {
        uint32_t binding;
        TextureHandle handle;
    };

public:
    MaterialHandle graphicsHandle;
    ShaderConst shader;
    std::string name;

    MaterialData(ShaderConst shader, const std::string_view name = "New Material");
    ~MaterialData();

    void UpdateLayout(bool updateVals = true);

    const uint32_t GetBufferSize() const { return data.size(); }
    const std::vector<std::byte> &GetData() const { return data; }
    const std::vector<TextureBinding> &GetTextureBindings() const { return textureBindings; }
    

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

    friend class Material;
};

class Material
{
public: 
    Ref<MaterialData> material{};

    Material() = default;
    Material(ShaderConst shader, const std::string_view name = "New Material");
    Material(const Ref<MaterialData>& _material) : material(_material) {}
    Material(Ref<MaterialData>&& _material) : material(std::move(_material)) {}
    ~Material();

    Material& operator=(const Ref<MaterialData>& _material)
    {
        material = _material;
        return *this;
    }

    Material& operator=(Ref<MaterialData>&& _material)
    {
        material = std::move(_material);
        return *this;
    }

    MaterialData* operator->() const { return material.get(); }
    operator bool() const { return material != nullptr; }
    operator Ref<MaterialData>&() { return material; }
    operator const Ref<MaterialData>&() const { return material; }

    inline void reset() noexcept { material.reset(); }



        inline void SetBool(const std::string &name, bool val, bool update = true) { 
        if(material != nullptr) {
            material->setIntegral<bool>(name, val);
            if(update) UpdateValues();
        }
    }

    template<typename T>
        requires std::integral<T> && (!std::same_as<T, bool>)
    inline void SetInt(const std::string &name, T val, bool update = true) {
        if(material != nullptr) {
            material->setIntegral<T>(name, val);
            if(update) UpdateValues();
        }
    }

    template<typename T>
        requires std::floating_point<T>
    inline void SetFloat(const std::string &name, T val, bool update = true) {
        if(material != nullptr) {
            material->setFloat<T>(name, val);
            if(update) UpdateValues();
        }
    }

    template<typename T>
        requires glm::type<T>::is_vec && std::integral<typename T::value_type>
    inline void SetVector(const std::string &name, T val, bool update = true) {
        if(material != nullptr) {
            material->setIntegral<T>(name, val);
            if(update) UpdateValues();
        }
    }

    template<typename T>
        requires glm::type<T>::is_vec && std::floating_point<typename T::value_type>
    inline void SetVector(const std::string &name, T val, bool update = true) {
        if(material != nullptr) {
            material->setFloat<T>(name, val);
            if(update) UpdateValues();
        }
    }

    inline void SetColor(const std::string &name, Color val, bool update = true) { 
        if(material != nullptr) {
            material->setFloat<glm::vec3>(name, (glm::vec3)val);
            if(update) UpdateValues();
        }
    }
    inline void SetColor3(const std::string &name, Color val, bool update = true) { 
        if(material != nullptr) {
            material->setFloat<glm::vec3>(name, (glm::vec3)val);
            if(update) UpdateValues();
        }
    }
    inline void SetColor4(const std::string &name, Color val, bool update = true) { 
        if(material != nullptr) {
            material->setFloat<glm::vec4>(name, (glm::vec4)val);
            if(update) UpdateValues();
        }
    }

    void SetMat2x2(std::string_view name, glm::mat2x2 val);
    void SetMat3x3(std::string_view name, glm::mat3x3 val);
    void SetMat4x4(std::string_view name, glm::mat4x4 val);

    void SetTexture(const std::string &name, TextureHandle handle);
    void SetTexture(const std::string &name, Texture texture);

    void UpdateValues();

    /// @brief Draw material properties in ImGui. Expects window to already be in progress.
    void DrawImGui();
private:
    bool drawImGuiParam(const ShaderParameter& param);

};


} // namespace graphics