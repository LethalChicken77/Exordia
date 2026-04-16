#pragma once
#include <cstdint>
#include <slang.h>
#include "graphics/api/shader_config.hpp"
#include "graphics/api/shader_layout.hpp"
#include "graphics/api/vertex_layout.hpp"
#include "graphics/api/handles.hpp"
#include "core/assets/shader_asset.hpp"

#include "glm/glm.hpp"
#include "primitives/color.hpp"
// #define __STDCPP_FLOAT16_T__
// #include <stdfloat>
using core::ShaderAsset;

namespace graphics
{
class Shader
{
public:
    Shader(ShaderAsset* vertAsset, ShaderAsset* fragAsset);
    Shader(ShaderAsset* vertAsset, ShaderAsset* fragAsset, const ShaderProperties& properties);
    ~Shader() = default;

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void Compile();
    void Update();

    const std::vector<uint32_t>& GetVertSpirv() const { return vertSpirv; }
    const std::vector<uint32_t>& GetFragSpirv() const { return fragSpirv; }
    const ShaderLayout& GetLayout() const { return layout; }
    const ShaderLayout* GetLayoutPtr() const { return &layout; }
    const VertexLayout& GetVertexLayout() const { return vertLayout; }

    void SetVertexShaderAsset(ShaderAsset* asset) { vertexShaderAsset = asset; }
    void SetFragmentShaderAsset(ShaderAsset* asset) { fragmentShaderAsset = asset; }

    ShaderProperties properties{};

    ShaderHandle graphicsHandle;

private:

    ShaderAsset* vertexShaderAsset = nullptr;
    ShaderAsset* fragmentShaderAsset = nullptr;
    std::vector<uint32_t> vertSpirv{};
    std::vector<uint32_t> fragSpirv{};
    ShaderLayout layout{};
    VertexLayout vertLayout{};
    bool isValid; // Used to prevent passing bad data to the graphics module if compilation fails

    void compileFrag();
    void compileVert();
};

class ComputeShader
{
public:
    ComputeShader(ShaderAsset& asset) 
        : shaderAsset(&asset) {}
    ~ComputeShader() = default;

    ComputeShader(const ComputeShader&) = delete;
    ComputeShader& operator=(const ComputeShader&) = delete;

    void Compile();

    const std::vector<uint32_t>& GetSpirv() const { return spirv; }
    const graphics::ShaderLayout &GetLayout() const { return layout; }

private:

    const ShaderAsset *shaderAsset;
    std::vector<uint32_t> spirv;
    ShaderLayout layout;
};

} // namespace graphics