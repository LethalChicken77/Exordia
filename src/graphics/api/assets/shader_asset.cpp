#include "shader_asset.hpp"

#include "asset.hpp"
#include "core/asset_manager.hpp"
#include "graphics/api/shader_config.hpp"
#include "modules.hpp"
#include "utils/debug.hpp"
#include "graphics/api/shader_compile.hpp"
#include "console.hpp"
#include "tui_helpers.hpp"

#include "TuiScript.h"

#include <memory>
#include <cmath>
#include <type_traits>
#include <slang-com-ptr.h>
#include <slang.h>

namespace graphics
{


void ShaderCodeAsset::Load() {
    if(!loadFile()) {
        m_status = core::AssetStatus::Invalid;
        return;
    }
    // unloadFile();
}

void ShaderCodeAsset::Unload() {
    unloadFile();
}

void parseProperties(tui::table table, ShaderProperties& props)
{
    table = table->getTable("properties");
    if(!table)
        return;

    TuiHelpers::ReadEnum(table, "shaderType", props.shaderType, ShaderType::Lit, ShaderType::UI);
    TuiHelpers::ReadEnum(table, "drawMode", props.drawMode, DrawMode::Fill, DrawMode::Points);
    TuiHelpers::ReadEnum(table, "blendMode", props.blendMode, BlendMode::Opaque, BlendMode::Premultiply);
    TuiHelpers::ReadEnum(table, "cullMode", props.cullMode, CullMode::None, CullMode::Both);
    TuiHelpers::ReadEnum(table, "depthTest", props.depthTest, CompareOp::Never, CompareOp::Always);
    TuiHelpers::ReadEnum(table, "depthWrite", props.depthWrite, DepthWrite::Auto, DepthWrite::Disabled);

    TuiHelpers::ReadBool(table, "alphaClipping", props.alphaClipping);
    TuiHelpers::ReadFloat(table, "alphaClippingThreshold", props.alphaClippingThreshold);
}

void ShaderAsset::Load() {
    m_status = core::AssetStatus::Loading;
    if(!loadFile()) {
        m_status = core::AssetStatus::Invalid;
        unloadFile();
        return;
    }
    
    std::string dataString = std::string(DataString());
    tui::table table = tui::toTable(TuiRef::loadString(dataString, "loadString"));

    // TODO: Support other fixed function shader setups (separate vert frag, mesh shaders, etc.)
    std::string meshFragPath = table->getString("meshFragShader");
    std::string vertFragPath = table->getString("vertFragShader");
    std::string vertPath = table->getString("vertShader");
    std::string meshPath = table->getString("meshShader");
    std::string fragPath = table->getString("fragShader");
    // Combined shaders prioritized, vertex shaders prioritized
    if(!vertFragPath.empty()) {
        setup = Setup::CombinedVertFrag;
        vertFragCode = core::AssetManager::LoadAsset<ShaderCodeAsset>(vertFragPath, true);
    }
    else if(!meshFragPath.empty()) {
        setup = Setup::CombinedMeshFrag;
        invalidate("Mesh shaders not supported (yet)");
        unloadFile();
        return;
    }
    else if(!vertPath.empty() && !fragPath.empty()) {
        setup = Setup::SeparateVertFrag;
        vertFragCode = core::AssetManager::LoadAsset<ShaderCodeAsset>(vertFragPath, true);
    }
    else if(!meshPath.empty() && !fragPath.empty()) {
        setup = Setup::SeparateMeshFrag;
        invalidate("Mesh shaders not supported (yet)");
        unloadFile();
        return;
    }
    else {
        m_status = core::AssetStatus::Invalid;
        Console::error("Invalid shader setup", "ShaderAsset");
    }

    parseProperties(table, properties);
    compile();
    if(m_status == core::AssetStatus::Invalid)
    {
        unloadFile();
        return;
    }
    
    graphicsModule->UpdateShader(sharedThis<ShaderAsset>());
    m_status = core::AssetStatus::Ready;
    unloadFile();
}

void ShaderAsset::Unload() {
    // graphicsModule->DeregisterShader(this);
    // unloadFile();
    meshFragCode.reset();
    vertFragCode.reset();
    vertCode.reset();
    meshCode.reset();
    fragCode.reset();
    m_status = core::AssetStatus::Unloaded;
}

void ShaderAsset::compile() {
    std::vector<uint32_t> tempVertSpirv{};
    std::vector<uint32_t> tempFragSpirv{};
    bool isValid = false;
    if(setup == Setup::CombinedVertFrag)
    {
        vertFragCode->Load();
        isValid = ShaderCompile::CompileSlangCombined(
            vertFragCode->Path(), 
            vertFragCode->DataString(), 
            &tempVertSpirv,
            &tempFragSpirv,
            &layout,
            &vertexLayout,
            &instanceLayout);
        
        // Console::log(layout.ToString());
    }
    else if (setup == Setup::SeparateVertFrag)
    {
        // vertexShaderCode->Load();
        // fragmentShaderCode->Load();
        // TODO: Compilation for split shaders
        // vertSpirv = vertexShaderCode->CompileSlang("vsMain", SLANG_STAGE_VERTEX);
        // fragSpirv = fragmentShaderCode->CompileSlang("fsMain", SLANG_STAGE_FRAGMENT);
    }
    if(isValid)
    {
        vertSpirv = tempVertSpirv;
        fragSpirv = tempFragSpirv;
        // layout = ShaderLayout(fragSpirv);
    }
    else {
        invalidate("Shader compilation or reflection failed");
    }
    // vertexShaderCode->Unload();
}

}; // namespace core