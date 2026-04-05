#pragma once
#include "core/asset.hpp"
#include "core/asset_data.hpp"

#include <slang.h>
#include <cstdint>
#include "glm/glm.hpp"
#include "primitives/color.hpp"
// #define __STDCPP_FLOAT16_T__
// #include <stdfloat>

namespace core
{

class ShaderAsset : public AssetData
{
public:
    static constexpr const char* className = "Shader";

    ShaderAsset(const ShaderAsset&) = delete;
    ShaderAsset& operator=(const ShaderAsset&) = delete;
    ShaderAsset(ShaderAsset&&) = delete;
    ShaderAsset& operator=(ShaderAsset&&) = delete;

    std::vector<uint32_t> CompileSlang(const std::string_view entryPointName, SlangStage slangStage) const; // TODO: Remove entirely
private:
    ShaderAsset(id_t newID) : AssetData(newID) {}
    friend class ObjectManager;
    friend class AssetManager;
};

} // namespace core