#pragma once
#include "core/asset.hpp"

#include <slang.h>
// #define __STDCPP_FLOAT16_T__
// #include <stdfloat>

namespace graphics
{

class ShaderCodeAsset : public core::Asset
{
public:
    static constexpr const std::string_view assetTypeName = "Shader Code";

    void Load() override;
    void Unload() override;

    void Compile();
private:

    friend class core::AssetManager;
};

using ShaderCode = Ref<ShaderCodeAsset>;

} // namespace core