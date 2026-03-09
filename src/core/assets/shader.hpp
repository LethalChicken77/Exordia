#pragma once
#include "core/asset.hpp"
#include "core/asset_data.hpp"

#include "graphics/api/shader_config.hpp"
#include "graphics/resources/shader_layout.hpp"

#include <slang.h>
#include "glm/glm.hpp"
#include "primitives/color.hpp"
#include <cstdint>
// #define __STDCPP_FLOAT16_T__
// #include <stdfloat>

namespace core
{
class Shader;
class ShaderAsset : public AssetData
{
public:
    static constexpr const char* className = "Shader";

    ShaderAsset(const ShaderAsset&) = delete;
    ShaderAsset& operator=(const ShaderAsset&) = delete;
    ShaderAsset(ShaderAsset&&) = delete;
    ShaderAsset& operator=(ShaderAsset&&) = delete;

    std::vector<uint32_t> CompileSlang(const char* moduleName, const char* entryPointName, SlangStage slangStage);
private:
    ShaderAsset(id_t newID) : AssetData(newID) {}
    friend class ObjectManager;
    friend class AssetManager;
    friend class Shader;
};

typedef std::variant<
    uint8_t,
    uint16_t,
    uint32_t,
    uint64_t,
    // std::float16_t,
    float,
    double,
    bool,
    glm::vec2,
    glm::vec3,
    glm::vec4,
    Color,
    glm::mat2,
    glm::mat3,
    glm::mat4
> ShaderValue;

class Shader : public Object
{
public:
    ~Shader() = default;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void Compile();

    const std::vector<uint32_t>& GetVertSpirv() const { return vertSpirv; }
    const std::vector<uint32_t>& GetFragSpirv() const { return fragSpirv; }
    const graphics::ShaderLayout &GetLayout() const { return layout; }

    void SetVertexShaderAsset(ShaderAsset* asset) { vertexShaderAsset = asset; }
    void SetFragmentShaderAsset(ShaderAsset* asset) { fragmentShaderAsset = asset; }

    graphics::ShaderProperties properties{};

private:
    Shader(id_t id) : Object(id) {}
    friend class ObjectManager;

    ShaderAsset* vertexShaderAsset = nullptr;
    ShaderAsset* fragmentShaderAsset = nullptr;
    std::vector<uint32_t> vertSpirv{};
    std::vector<uint32_t> fragSpirv{};
    graphics::ShaderLayout layout;

    void compileFrag();
    void compileVert();
};

class ComputeShader : public Object
{
public:
    ~ComputeShader() = default;

    ComputeShader(const ComputeShader&) = delete;
    ComputeShader& operator=(const ComputeShader&) = delete;

    void Compile();

    const std::vector<char>& GetSpirv() const { return spirv; }
    const graphics::ShaderLayout &GetLayout() const { return layout; }

private:
    ComputeShader(ShaderAsset& asset, id_t id) 
        : Object(id), shaderAsset(asset) {}
    friend class ObjectManager;

    ShaderAsset& shaderAsset;
    std::vector<char> spirv;
    graphics::ShaderLayout layout;
};

} // namespace core