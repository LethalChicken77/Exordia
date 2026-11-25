#include "shader_asset.hpp"
#include <slang-com-ptr.h>
#include <slang.h>
#include "utils/debug.hpp"
#include "graphics/api/shader_compile.hpp"

namespace graphics
{

void ShaderCodeAsset::Load()
{
    if(!loadFile())
    {
        m_status = core::AssetStatus::Invalid;
        return;
    }

    // unloadFile();
}

void ShaderCodeAsset::Unload()
{
    unloadFile();
}

}; // namespace core