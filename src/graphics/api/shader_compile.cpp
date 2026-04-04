#include "shader_compile.hpp"
#include "console.hpp"
#include "debug.hpp"

namespace graphics
{

void ShaderCompile::init()
{
    if(s_initialized) return;
    SlangGlobalSessionDesc globalDesc = {};
    if (SLANG_FAILED(createGlobalSession(&globalDesc, s_globalSession.writeRef())))
        throw std::runtime_error("Slang: failed to create global session");

    // Select profiles
    s_spirvProfile = s_globalSession->findProfile("sm_6_8");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_7");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_6");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_5");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_4");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_3");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_2");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_1");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_6_0");
    if (!s_spirvProfile)
        s_spirvProfile = s_globalSession->findProfile("sm_5_1");
    if(!s_spirvProfile)
        throw std::runtime_error("[Error] [ShaderCompile] No suitable Slang profile found.");
}

std::vector<uint32_t> ShaderCompile::CompileSlang(
        const std::string_view path,
        const std::string_view source,
        const std::string_view moduleName,
        const std::string_view entryPointName,
        SlangStage slangStage,
        ShaderLayout* layout,
        VertexLayout* vertLayout)
{
    Console::logf("Compiling shader {} as {} shader", path, Debug::SlangStageToString((uint32_t)slangStage), "ShaderAsset");

    init();

    slang::SessionDesc sessionDesc = {};
    // const char* paths[] = { "C:/VulkanSDK/1.4.309.0/Include/slang" }; // adjust as needed
    const char* paths[] = { "./internal/shaders", "./assets" }; // Set search paths
    sessionDesc.searchPaths = paths;
    sessionDesc.searchPathCount = 2;
    sessionDesc.compilerOptionEntryCount = options.size(); 
    sessionDesc.compilerOptionEntries = options.data();

    Slang::ComPtr<slang::ISession> session;
    if (SLANG_FAILED(s_globalSession->createSession(sessionDesc, session.writeRef())))
        throw std::runtime_error("Slang: failed to create session");

    Slang::ComPtr<slang::ICompileRequest> request;
    if (SLANG_FAILED(session->createCompileRequest(request.writeRef())))
        throw std::runtime_error("Slang: failed to create compile request");

    request->addCodeGenTarget(SLANG_SPIRV);

    request->setTargetProfile(0, s_spirvProfile);

    int tuIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, moduleName.data());
    if (tuIndex < 0) throw std::runtime_error("Slang: addTranslationUnit failed");
    request->addTranslationUnitSourceString(tuIndex, moduleName.data(), source.data());

    int entryPointIndex = request->addEntryPoint(tuIndex, entryPointName.data(), slangStage);
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

    // SpirvReflectTest(spirv);

    return spirv;
}

} // namespace graphics