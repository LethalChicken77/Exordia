#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include "graphics/resources/layouts/shader_layout.hpp"
#include "graphics/resources/layouts/vertex_layout.hpp"

namespace graphics
{

class SlangReflect
{
public:
    /// @brief Create layouts from reflection data returned by Slang compiler
    /// @param request Compile request. Ensure this has completed successfully!
    /// @param vertEntryPoint Vertex shader entrypoint index. -1 for invalid.
    /// @param fragEntryPoint Fragment shader entrypoint index. -1 for invalid.
    /// @param globalLayout Target global layout. Pass nullptr to not generate.
    /// @param vertLayout Target vertex layout. Pass nullptr to not generate.
    static void GenerateLayouts(Slang::ComPtr<slang::ICompileRequest> request, 
        int vertEntryPoint,
        int fragEntryPoint,
        ShaderLayout* globalLayout,
        VertexLayout* vertLayout, 
        uint32_t stageFlags);

private:
    static uint32_t slangStageToVkStage(SlangStage stage);
    static TypeDescription reflectNumeric(slang::TypeReflection* type);
    static TypeDescription reflectArray(slang::VariableLayoutReflection* field);
    static std::vector<ShaderParameter> reflectStruct(slang::VariableLayoutReflection* fields, const std::string& parentName, uint32_t baseOffset);

    static BufferLayout reflectBuffer(slang::VariableLayoutReflection* fields);
    static void reflectLayout(slang::VariableLayoutReflection* reflect, ShaderLayout* globalLayout, uint32_t stageFlags);


    static bool handleVertexField(slang::VariableLayoutReflection* field, VertexLayout::Attribute& attribute);
    static void reflectVertex(slang::EntryPointReflection* reflect, VertexLayout* vertLayout);

    template<typename T>
    static bool readDefaultScalar(const void* buffer, size_t bufferSize, T& value);
    template<typename T, glm::length_t N>
    static bool readDefaultVector(const void* buffer, size_t bufferSize, glm::vec<N, T, glm::defaultp>& value);
    template<typename T, typename Variant>
    static bool decodeDefaultScalarOrVector(const void* buffer, size_t bufferSize, size_t elementCount, Variant& out);
    template<typename Variant>
    static bool decodeDefaultValue(slang::TypeReflection* type, const void* buffer, size_t bufferSize, Variant& out);
};


// Obsolete with slang reflect, but might as well include it in case I need it.
// TODO: Move old spirv reflect code here
class SpirvReflect
{
public:
};

};