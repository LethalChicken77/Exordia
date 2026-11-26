#pragma once
#include "core/asset.hpp"
#include "graphics/api/handles.hpp"
#include "graphics/api/shader_config.hpp"
#include "graphics/resources/layouts/shader_layout.hpp"
#include "graphics/resources/layouts/vertex_layout.hpp"

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

private:

    friend class core::AssetManager;
};
using ShaderCode = Ref<ShaderCodeAsset>;

class ShaderAsset : public core::Asset
{
public:
    static constexpr const std::string_view assetTypeName = "Shader";

    void Load() override;
    void Unload() override;

    const std::vector<uint32_t>& GetVertSpirv() const { return vertSpirv; }
    const std::vector<uint32_t>& GetFragSpirv() const { return fragSpirv; }
    const ShaderLayout& GetLayout() const { return layout; }
    const ShaderLayout* GetLayoutPtr() const { return &layout; }
    const VertexLayout& GetVertexLayout() const { return vertexLayout; }
    const VertexLayout& GetInstanceLayout() const { return instanceLayout; }

    ShaderHandle graphicsHandle{};
    ShaderProperties properties{};
private:
    enum class Setup
    {
        Invalid,
        SeparateVertFrag,
        CombinedVertFrag,
        SeparateMeshFrag,
        CombinedMeshFrag,
        // IDEA: Support geo and tess shaders, probably not needed
    };

    Setup setup{};

    ShaderCode vertCode{};
    ShaderCode meshCode{};
    ShaderCode fragCode{};
    ShaderCode vertFragCode{};
    ShaderCode meshFragCode{};

    std::vector<uint32_t> vertSpirv{};
    std::vector<uint32_t> meshSpirv{};
    std::vector<uint32_t> fragSpirv{};

    // Shader layouts
    ShaderLayout layout{};
    VertexLayout vertexLayout{};
    VertexLayout instanceLayout{};

    void compile();

    friend class core::AssetManager;
};

using Shader = Ref<ShaderAsset>; // IDEA: Wrap ref for helper methods
using ShaderConst = Ref<const ShaderAsset>;

} // namespace core