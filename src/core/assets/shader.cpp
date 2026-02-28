#include "shader.hpp"
#include <slang-com-ptr.h>
#include "spirv_reflect.h"
#include <slang.h>

namespace core
{

std::vector<uint32_t> ShaderAsset::CompileSlang(const char* moduleName, const char* entryPointName, SlangStage slangStage)
{
    std::string source(data.begin(), data.end());

    Slang::ComPtr<slang::IGlobalSession> globalSession;
    SlangGlobalSessionDesc globalDesc = {};
    if (SLANG_FAILED(createGlobalSession(&globalDesc, globalSession.writeRef())))
        throw std::runtime_error("Slang: failed to create global session");

    std::vector<slang::CompilerOptionEntry> options{};
    // TODO: Only enable if scalar block layout extension is available
    options.push_back({ slang::CompilerOptionName::GLSLForceScalarLayout, slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 1} });
    options.push_back({ slang::CompilerOptionName::VulkanUseGLLayout, slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 0} });
    // options.push_back({ slang::CompilerOptionName::VulkanEmitReflection, slang::CompilerOptionValue{slang::CompilerOptionValueKind::Int, 1} });

    slang::SessionDesc sessionDesc = {};
    // const char* paths[] = { "C:/VulkanSDK/1.4.309.0/Include/slang" }; // adjust as needed
    const char* paths[] = { "./internal/shaders", "./assets/shaders" }; // Set search paths
    sessionDesc.searchPaths = paths;
    sessionDesc.searchPathCount = 2;
    sessionDesc.compilerOptionEntryCount = options.size(); 
    sessionDesc.compilerOptionEntries = options.data();

    Slang::ComPtr<slang::ISession> session;
    if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
        throw std::runtime_error("Slang: failed to create session");

    Slang::ComPtr<slang::ICompileRequest> request;
    if (SLANG_FAILED(session->createCompileRequest(request.writeRef())))
        throw std::runtime_error("Slang: failed to create compile request");

    request->addCodeGenTarget(SLANG_SPIRV);

    SlangProfileID spirvProfile = globalSession->findProfile("sm_6_8");
    if (!spirvProfile)
        throw std::runtime_error("Slang: sm_6_8 profile not available");
    request->setTargetProfile(0, spirvProfile);

    int tuIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, moduleName);
    if (tuIndex < 0) throw std::runtime_error("Slang: addTranslationUnit failed");
    request->addTranslationUnitSourceString(tuIndex, moduleName, source.c_str());

    int entryPointIndex = request->addEntryPoint(tuIndex, entryPointName, slangStage);
    if (entryPointIndex < 0) throw std::runtime_error("Slang: addEntryPoint failed");

    if (SLANG_FAILED(request->compile()))
    {
        const char* diag = request->getDiagnosticOutput();
        // getDiagnosticOutput may provide (const char**, size_t*) or a single-string API depending on build
        if(diag != nullptr)
            Console::error("Shader compile failed: " + std::string(diag), "Slang");
        else
            Console::error("Shader compile failed (no info)", "Slang");
        return {};
    }

    slang::IBlob* rawBlob = nullptr;
    if (SLANG_FAILED(request->getEntryPointCodeBlob(entryPointIndex, 0, &rawBlob)))
        throw std::runtime_error("Slang: failed to get SPIR-V code blob");

    Slang::ComPtr<slang::IBlob> spirvBlob;
    spirvBlob.attach(rawBlob); // take ownership

    const void* data = spirvBlob->getBufferPointer();
    size_t sizeBytes = spirvBlob->getBufferSize();
    if (!data || sizeBytes == 0)
        throw std::runtime_error("Slang: empty SPIR-V output");

    if (sizeBytes % sizeof(uint32_t) != 0)
        throw std::runtime_error("Slang: SPIR-V size not a multiple of 4");

    const uint32_t* words = reinterpret_cast<const uint32_t*>(data);
    size_t wordCount = sizeBytes / sizeof(uint32_t);
    std::vector<uint32_t> spirv(words, words + wordCount);

    SpirvReflectTest(spirv);

    return spirv;
}

void ShaderAsset::SpirvReflectTest(const std::vector<uint32_t> &spirv)
{
    spv_reflect::ShaderModule module(spirv);

    uint32_t bindingCount = 0;
    SpvReflectResult result = module.EnumerateDescriptorBindings(&bindingCount, nullptr);
    if(result != SPV_REFLECT_RESULT_SUCCESS)
    {
        Console::error(std::format("Failed to enumerate descriptor bindings: {}", (uint32_t)result), "ShaderAsset");
        return;
    }
    
    if(bindingCount == 0)
    {
        Console::log("No descriptor bindings found.", "ShaderAsset");
        return;
    }

    std::vector<SpvReflectDescriptorBinding*> bindings(bindingCount);
    result = module.EnumerateDescriptorBindings(&bindingCount, bindings.data());
    if(result != SPV_REFLECT_RESULT_SUCCESS)
    {
        Console::error(std::format("Failed to retrieve descriptor bindings: {}", (uint32_t)result), "ShaderAsset");
        return;
    }

    bool foundMatInfo = false;
    for(SpvReflectDescriptorBinding* binding : bindings)
    {
        if(binding && strcmp(binding->name, "materialInfo") == 0)
        {
            foundMatInfo = true;
            Console::logf("materialInfo", "ShaderAsset", true);
            Console::logf("Descriptor Set : {}", binding->set, "", false);
            Console::logf("Binding        : {}", binding->binding);
            Console::logf("Descriptor Type: {}", (uint32_t)binding->descriptor_type);
            Console::logf("Block size     : {}", binding->block.size);

            const SpvReflectBlockVariable& block = binding->block;
            Console::log(std::format("Fields ({}):", block.member_count));

            for(uint32_t m = 0; m < block.member_count; m++)
            {
                const SpvReflectBlockVariable member = block.members[m];
                
                std::string memberString = std::format("\t{} \toffset = {} \tsize = {}", member.name, member.offset, member.size);
                if(member.type_description && member.type_description->type_name)
                {
                    memberString += std::format("\t\ttype: {}", member.type_description->type_name);
                }
                Console::log(memberString);
            }
        }
    }
}

void Shader::Compile()
{
    vertSpirv = vertexShaderAsset->CompileSlang("vertexShader", "vsMain", SLANG_STAGE_VERTEX);
    fragSpirv = fragmentShaderAsset->CompileSlang("fragmentShader", "fsMain",  SLANG_STAGE_FRAGMENT);

    // layout = graphics::ShaderLayout(vertSpirv, "materialInfo");
}

}; // namespace core