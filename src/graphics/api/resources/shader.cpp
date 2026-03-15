#include "shader.hpp"
#include <slang-com-ptr.h>
#include "spirv_reflect.h"
#include <slang.h>
#include "utils/debug.hpp"

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
    vertSpirv = vertexShaderAsset->CompileSlang("vertexShader", "vsMain", SLANG_STAGE_VERTEX);
    fragSpirv = fragmentShaderAsset->CompileSlang("fragmentShader", "fsMain",  SLANG_STAGE_FRAGMENT);

    layout = graphics::BufferLayout(fragSpirv, "materialInfo");
}

}; // namespace core