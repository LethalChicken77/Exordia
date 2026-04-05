#include "shader.hpp"
#include <slang-com-ptr.h>
#include "spirv_reflect.h"
#include <slang.h>
#include "utils/debug.hpp"
#include "graphics/api/shader_compile.hpp"

namespace graphics
{

Shader::Shader(const ShaderAsset* vertAsset, const ShaderAsset* fragAsset)
{
    vertexShaderAsset = vertAsset;
    fragmentShaderAsset = fragAsset;
    Compile();
}

void Shader::Compile()
{
    if(vertexShaderAsset == fragmentShaderAsset) // usually true
    {
        ShaderCompile::CompileSlangCombined(
            vertexShaderAsset->GetPath(), 
            vertexShaderAsset->GetDataString(), 
            &vertSpirv,
            &fragSpirv,
            nullptr,
            nullptr);
    }
    else
    {
        vertSpirv = vertexShaderAsset->CompileSlang("vsMain", SLANG_STAGE_VERTEX);
        fragSpirv = fragmentShaderAsset->CompileSlang("fsMain", SLANG_STAGE_FRAGMENT);
    }

    vertLayout = VertexLayout(vertSpirv); // TODO: Replace with slang reflection
    layout = ShaderLayout(fragSpirv);
}

void ComputeShader::Compile()
{
    spirv = shaderAsset->CompileSlang("main", SLANG_STAGE_VERTEX);

    layout = ShaderLayout(spirv);
}

}; // namespace core