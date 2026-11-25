#include "shader.hpp"
#include <slang-com-ptr.h>
#include "spirv_reflect.h"
#include <slang.h>
#include "utils/debug.hpp"
#include "graphics/api/shader_compile.hpp"
#include "modules.hpp"

namespace graphics
{

Shader::Shader(Ref<ShaderCodeAsset> vertAsset, Ref<ShaderCodeAsset> fragAsset)
    : vertexShaderCode(vertAsset),
    fragmentShaderCode(fragAsset)
{
    Compile();
}

Shader::Shader(Ref<ShaderCodeAsset> vertAsset, Ref<ShaderCodeAsset> fragAsset, const ShaderProperties& _properties)
    : vertexShaderCode(vertAsset),
    fragmentShaderCode(fragAsset),
    properties(_properties)
{
    Compile();
}

void Shader::Compile()
{
    std::vector<uint32_t> tempVertSpirv{};
    std::vector<uint32_t> tempFragSpirv{};
    isValid = false;
    if(vertexShaderCode == fragmentShaderCode) // usually true
    {
        vertexShaderCode->Load();
        isValid = ShaderCompile::CompileSlangCombined(
            vertexShaderCode->Path(), 
            vertexShaderCode->DataString(), 
            &tempVertSpirv,
            &tempFragSpirv,
            &layout,
            &vertexLayout,
            &instanceLayout);
            
        // Console::log(layout.ToString());
    }
    else
    {
        vertexShaderCode->Load();
        fragmentShaderCode->Load();
        // TODO: Compilation for split shaders
        // vertSpirv = vertexShaderCode->CompileSlang("vsMain", SLANG_STAGE_VERTEX);
        // fragSpirv = fragmentShaderCode->CompileSlang("fsMain", SLANG_STAGE_FRAGMENT);
    }
    if(isValid)
    {
        vertSpirv = tempVertSpirv;
        fragSpirv = tempFragSpirv;
        // layout = ShaderLayout(fragSpirv);
        graphicsModule.UpdateShader(*this);
    }
    vertexShaderCode->Unload();
    fragmentShaderCode->Unload();
}

void Shader::Update()
{
    if(isValid)
        graphicsModule.UpdateShader(*this);
}

void ComputeShader::Compile()
{
    // spirv = ShaderCodeAsset->CompileSlang("main", SLANG_STAGE_COMPUTE);
}

}; // namespace core