#pragma once
#include "core/asset.hpp"
#include "core/asset_data.hpp"

#include "graphics/shader_config.hpp"

#include <slang.h>

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

class Shader : public Object
{
public:
    ~Shader() = default;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void Compile();

    const std::vector<uint32_t>& GetVertSpirv() const { return vertSpirv; }
    const std::vector<uint32_t>& GetFragSpirv() const { return fragSpirv; }

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

private:
    ComputeShader(ShaderAsset& asset, id_t id) 
        : Object(id), shaderAsset(asset) {}
    friend class ObjectManager;

    ShaderAsset& shaderAsset;
    std::vector<char> spirv;
};

} // namespace core