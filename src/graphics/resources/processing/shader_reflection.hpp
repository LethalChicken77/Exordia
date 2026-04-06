#pragma once
#include <slang.h>
#include <slang-com-ptr.h>
#include "graphics/api/shader_layout.hpp"
#include "graphics/api/vertex_layout.hpp"

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
        VertexLayout* vertLayout);

private:
    static void reflectLayout(slang::VariableLayoutReflection* reflect, ShaderLayout* globalLayout);
    static void reflectVertex(slang::EntryPointReflection* reflect, VertexLayout* vertLayout);
};


// Obsolete with slang reflect, but might as well include it in case I need it.
// TODO: Move old spirv reflect code here
class SpirvReflect
{
public:
};

};