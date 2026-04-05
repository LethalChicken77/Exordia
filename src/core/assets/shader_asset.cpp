#include "shader_asset.hpp"
#include <slang-com-ptr.h>
#include <slang.h>
#include "utils/debug.hpp"
#include "graphics/api/shader_compile.hpp"

namespace core
{

std::vector<uint32_t> ShaderAsset::CompileSlang(const std::string_view entryPointName, SlangStage slangStage) const
{
    return graphics::ShaderCompile::CompileSlang(*this, entryPointName, slangStage, nullptr, nullptr);
}

}; // namespace core