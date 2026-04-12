#include "shader.hpp"
#include <slang-com-ptr.h>
#include "spirv_reflect.h"
#include <slang.h>
#include "utils/debug.hpp"
#include "graphics/api/shader_compile.hpp"
#include "modules.hpp"

namespace graphics
{

Shader::Shader(ShaderAsset* vertAsset, ShaderAsset* fragAsset)
    : vertexShaderAsset(vertAsset),
    fragmentShaderAsset(fragAsset)
{
    Compile();
}

Shader::Shader(ShaderAsset* vertAsset, ShaderAsset* fragAsset, const ShaderProperties& _properties)
    : vertexShaderAsset(vertAsset),
    fragmentShaderAsset(fragAsset),
    properties(_properties)
{
    Compile();
}

void Shader::Compile()
{
    vertLayout = VertexLayout();
    std::vector<uint32_t> tempVertSpirv{};
    std::vector<uint32_t> tempFragSpirv{};
    isValid = false;
    if(vertexShaderAsset == fragmentShaderAsset) // usually true
    {
        vertexShaderAsset->LoadData();
        isValid = ShaderCompile::CompileSlangCombined(
            vertexShaderAsset->GetPath(), 
            vertexShaderAsset->GetDataString(), 
            &tempVertSpirv,
            &tempFragSpirv,
            nullptr,
            &vertLayout);
    }
    else
    {
        vertexShaderAsset->LoadData();
        fragmentShaderAsset->LoadData();
        // TODO: Migrate to call ShaderCompile directly
        vertSpirv = vertexShaderAsset->CompileSlang("vsMain", SLANG_STAGE_VERTEX);
        fragSpirv = fragmentShaderAsset->CompileSlang("fsMain", SLANG_STAGE_FRAGMENT);
    }
    if(isValid)
    {
        vertSpirv = tempVertSpirv;
        fragSpirv = tempFragSpirv;
        // vertLayout = VertexLayout(vertSpirv); // TODO: Replace with slang reflection
        layout = ShaderLayout(fragSpirv);
        graphicsModule.UpdateShader(*this);
    }
}

void Shader::Update()
{
    if(isValid)
        graphicsModule.UpdateShader(*this);
}

void ComputeShader::Compile()
{
    spirv = shaderAsset->CompileSlang("main", SLANG_STAGE_VERTEX);

    layout = ShaderLayout(spirv);
}

}; // namespace core