#pragma once
#include <string_view>
#include "resources/shader.hpp"

namespace graphics
{

class ShaderCompile
{
public:

    /// @brief Compile Slang source code to SPIR-V.
    /// @param path Path to the shader file. Only used for debug strings.
    /// @param source Raw source text, passed as a string.
    /// @param moduleName 
    /// @param entryPointName 
    /// @param slangStage 
    /// @param layout Shader layout generated from code. Returns uniform buffers, textures, and samplers used by the shader.
    /// @param vertLayout Vertex layout generated from code. Only use on vertex shaders.
    /// @return SPIR-V code
    /// @note Also handles HLSL, with some minor caveats.
    static std::vector<uint32_t> CompileSlang(
        const std::string_view path,
        const std::string_view source,
        const std::string_view moduleName,
        const std::string_view entryPointName,
        SlangStage slangStage,
        ShaderLayout* layout,
        VertexLayout* vertLayout);
    
    inline static std::vector<uint32_t> CompileSlang(
        const ShaderAsset& shaderAsset,
        const std::string_view moduleName,
        const std::string_view entryPointName,
        SlangStage slangStage,
        ShaderLayout* layout,
        VertexLayout* vertLayout)
    {
        const std::span<const char> data = shaderAsset.getData();
        const std::string_view dataView = std::string_view(data.data(), data.size());
        return CompileSlang(
            shaderAsset.getPath(), 
            dataView,
            moduleName,
            entryPointName,
            slangStage,
            layout,
            vertLayout
        );
    }

    // TODO: Support GLSL

private:
    void slangReflect(ShaderLayout* layout, VertexLayout vertLayout);
    void slangReflectLayout(slang::ProgramLayout& reflect, ShaderLayout* layout);
    void slangReflectVertex(slang::ProgramLayout& reflect, VertexLayout vertLayout);
};
    
};