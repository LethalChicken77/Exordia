#pragma once
#include <string_view>
#include <slang.h>
#include <slang-com-ptr.h>
#include "resources/shader.hpp"

namespace graphics
{

class ShaderCompile
{
public:
    static inline std::vector<slang::CompilerOptionEntry> options{ 
        { slang::CompilerOptionName::GLSLForceScalarLayout, slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 1}},
        { slang::CompilerOptionName::VulkanUseGLLayout, slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 0}},
        { slang::CompilerOptionName::PreserveParameters, slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 1}}
    };

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
        const std::string dataString = std::string(data.data(), data.size()); // TODO: Remove copy to string (Needed for null termination in string view)
        return CompileSlang(
            shaderAsset.getPath(), 
            dataString,
            moduleName,
            entryPointName,
            slangStage,
            layout,
            vertLayout
        );
    }

    // TODO: Support GLSL

private:
    static inline bool s_initialized = false;
    static inline Slang::ComPtr<slang::IGlobalSession> s_globalSession{};
    static inline SlangProfileID s_spirvProfile;

    static void init();
    static void slangReflect(ShaderLayout* layout, VertexLayout vertLayout);
    static void slangReflectLayout(slang::ProgramLayout& reflect, ShaderLayout* layout);
    static void slangReflectVertex(slang::ProgramLayout& reflect, VertexLayout vertLayout);
};
    
};