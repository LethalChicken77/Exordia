#include <filesystem>
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
        const std::string_view entryPointName,
        SlangStage slangStage,
        ShaderLayout* layout,
        VertexLayout* vertLayout)
{
    Console::logf("Compiling shader {} as {} shader", path, Debug::SlangStageToString((uint32_t)slangStage), "ShaderAsset");

    init();
    Slang::ComPtr<slang::ISession> session = createSlangSession();

    Slang::ComPtr<slang::ICompileRequest> request;
    if (SLANG_FAILED(session->createCompileRequest(request.writeRef())))
        throw std::runtime_error("Slang: failed to create compile request");

    request->addCodeGenTarget(SLANG_SPIRV); // TODO: Make configurable for debug
    request->setTargetProfile(0, s_spirvProfile);

    std::string moduleName = createModuleName(path);
    int tuIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, moduleName.c_str());
    if (tuIndex < 0) 
        throw std::runtime_error("Slang: addTranslationUnit failed");
    request->addTranslationUnitSourceString(tuIndex, moduleName.c_str(), source.data());

    
    int entryPointIndex = request->addEntryPoint(tuIndex, entryPointName.data(), slangStage);
    if (entryPointIndex < 0) 
        throw std::runtime_error("Slang: addEntryPoint failed");


    SlangResult compileResult = request->compile();
    const char* diag = request->getDiagnosticOutput();
    if (SLANG_FAILED(compileResult))
    {
        std::string msg = "Slang: shader compilation failed";
        if (diag && *diag != '\0')
            msg += "\n" + std::string(diag);

        Console::error(msg, "Slang");
        return {};
    }
    if(diag && *diag != '\0')
    {
        Console::warn("Slang compilation warnings:\n" + std::string(diag), "Slang");
    }

    
    std::vector<uint32_t> spirv = getSpirv(request, entryPointIndex);

    return spirv;
}

bool ShaderCompile::CompileSlangCombined(
        const std::string_view path,
        const std::string_view source,
        std::vector<uint32_t>* vertexDest,
        std::vector<uint32_t>* fragmentDest,
        ShaderLayout* layout,
        VertexLayout* vertLayout)
{
    Console::logf("Compiling combined shader {}", path, "ShaderCompile");

    init();
    Slang::ComPtr<slang::ISession> session = createSlangSession();

    Slang::ComPtr<slang::ICompileRequest> request;
    if (SLANG_FAILED(session->createCompileRequest(request.writeRef())))
        throw std::runtime_error("Slang: failed to create compile request");

    request->addCodeGenTarget(SLANG_SPIRV); // TODO: Make configurable for debug
    request->setTargetProfile(0, s_spirvProfile);

    std::string moduleName = createModuleName(path);
    int tuIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, moduleName.c_str());
    if (tuIndex < 0) 
        throw std::runtime_error("Slang: addTranslationUnit failed");
    request->addTranslationUnitSourceString(tuIndex, moduleName.c_str(), source.data());

    int vertEP = request->addEntryPoint(tuIndex, "vsMain", SLANG_STAGE_VERTEX);
    int fragEP = request->addEntryPoint(tuIndex, "fsMain", SLANG_STAGE_FRAGMENT);
    if (vertEP < 0 || fragEP < 0)
        throw std::runtime_error("Slang: addEntryPoint failed");
    
    SlangResult compileResult = request->compile();
    const char* diag = request->getDiagnosticOutput();
    if (SLANG_FAILED(compileResult))
    {
        std::string msg = "Slang: shader compilation failed";
        if (diag && *diag != '\0')
            msg += "\n" + std::string(diag);

        Console::error(msg, "Slang");
        return {};
    }
    if(diag && *diag != '\0')
    {
        Console::warn("Slang compilation warnings:\n" + std::string(diag), "Slang");
    }

    getSpirvInPlace(request, vertEP, vertexDest);
    getSpirvInPlace(request, fragEP, fragmentDest);

    return true;
}

Slang::ComPtr<slang::ISession> ShaderCompile::createSlangSession()
{
    slang::SessionDesc sessionDesc = {};
    sessionDesc.searchPaths = searchPaths.data();
    sessionDesc.searchPathCount = searchPaths.size();
    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = options.size(); 
    Slang::ComPtr<slang::ISession> session;
    if (SLANG_FAILED(s_globalSession->createSession(sessionDesc, session.writeRef())))
        throw std::runtime_error("Slang: failed to create session");
    return session;
}

std::string ShaderCompile::createModuleName(const std::string_view _path)
{
    std::filesystem::path path{_path};
    return path.stem().string();
}

void ShaderCompile::getSpirvInPlace(Slang::ComPtr<slang::ICompileRequest> request, int entryPointIndex, std::vector<uint32_t>* dest)
{
    // Get blob
    Slang::ComPtr<slang::IBlob> spirvBlob;
    if (SLANG_FAILED(request->getEntryPointCodeBlob(entryPointIndex, 0, spirvBlob.writeRef())))
        throw std::runtime_error("Slang: failed to get SPIR-V code blob");

    const void* data = spirvBlob->getBufferPointer();
    size_t sizeBytes = spirvBlob->getBufferSize();

    if (!data || sizeBytes == 0)
        throw std::runtime_error("Slang: empty SPIR-V output");
    if (sizeBytes % sizeof(uint32_t) != 0)
        throw std::runtime_error("Slang: SPIR-V size not a multiple of 4");

    const uint32_t* words = static_cast<const uint32_t*>(data);
    size_t wordCount = sizeBytes / sizeof(uint32_t);
    *dest = std::vector<uint32_t>(words, words + wordCount);
}

} // namespace graphics