#pragma once
#include <string_view>
#include <slang.h>
#include <slang-com-ptr.h>
#include "assets/shader_asset.hpp"

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
    static inline std::vector<const char*> searchPaths{
        VULKAN_SDK "/Bin/slang-standard-module-2026.1",
        "./internal/shaders",
        "./assets"
    };

    /// @brief Compile Slang source code to SPIR-V.
    /// @param path Path to the shader file. Only used for debug strings.
    /// @param source Raw source text, passed as a string.
    /// @param moduleName 
    /// @param entryPointName 
    /// @param slangStage 
    /// @param globalLayout Shader layout generated from code. Returns uniform buffers, textures, and samplers used by the shader.
    /// @param vertLayout Vertex layout generated from code. Only use on vertex shaders.
    /// @param instanceLayout Instance layout generated from code. Only use on vertex shaders.
    /// @return SPIR-V code
    /// @note Also handles HLSL, with some minor caveats.
    static std::vector<uint32_t> CompileSlang(
        const std::string_view path,
        const std::string_view source,
        const std::string_view entryPointName,
        SlangStage slangStage,
        ShaderLayout* globalLayout,
        VertexLayout* vertLayout,
        VertexLayout* instanceLayout);
    
    inline static std::vector<uint32_t> CompileSlang(
        const ShaderCodeAsset& shaderAsset,
        const std::string_view entryPointName,
        SlangStage slangStage,
        ShaderLayout* globalLayout,
        VertexLayout* vertLayout,
        VertexLayout* instanceLayout)
    {
        std::string_view data = shaderAsset.DataString();
        const std::string dataString = std::string(data); // TODO: Remove copy to string (Needed for null termination in string view)
        return CompileSlang(
            shaderAsset.Path(), 
            dataString,
            entryPointName,
            slangStage,
            globalLayout,
            vertLayout,
            instanceLayout
        );
    }

    /// @brief Compile a single-file vertex and fragment shader.
    /// @param path Path to the shader file. Only used for debug strings.
    /// @param source Raw source text, passed as a string.
    /// @param vertexDest Location to put the resulting vertex SPIR-V.
    /// @param fragmentDest Location to put the resulting fragment SPIR-V.
    /// @param globalLayout Shader layout generated from code. Returns uniform buffers, textures, and samplers used by the shader.
    /// @param vertLayout Vertex layout generated from code. Only use on vertex shaders.
    /// @return True on success, false on failure. Throws on unhandled error.
    /// @note For separate shaders, use CompileSlang. 
    static bool CompileSlangCombined(
        const std::string_view path,
        const std::string_view source,
        std::vector<uint32_t>* vertexDest,
        std::vector<uint32_t>* fragmentDest,
        ShaderLayout* globalLayout,
        VertexLayout* vertLayout,
        VertexLayout* instanceLayout);

    // TODO: Support GLSL

private:
    static inline bool s_initialized = false;
    static inline Slang::ComPtr<slang::IGlobalSession> s_globalSession{};
    static inline SlangProfileID s_spirvProfile;

    static void init();
    static Slang::ComPtr<slang::ISession> createSlangSession();
    static std::string createModuleName(const std::string_view path);
    static void getSpirvInPlace(Slang::ComPtr<slang::ICompileRequest> request, int entryPointIndex, std::vector<uint32_t>* dest);
    static inline std::vector<uint32_t> getSpirv(Slang::ComPtr<slang::ICompileRequest> request, int entryPointIndex)
    {
        std::vector<uint32_t> result{};
        getSpirvInPlace(request, entryPointIndex, &result);
        return result;
    }
};
    
};