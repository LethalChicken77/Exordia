#include "shader_reflection.hpp"
#include <console.hpp>

namespace graphics
{


void SlangReflect::GenerateLayouts(
    Slang::ComPtr<slang::ICompileRequest> request, 
    int vertEntryPoint, 
    int fragEntryPoint, 
    ShaderLayout* globalLayout, 
    VertexLayout* vertLayout)
{
    if(globalLayout == nullptr && vertLayout == nullptr) return; // No work to do

    Slang::ComPtr<slang::IComponentType> reflection;
    request->getProgramWithEntryPoints(reflection.writeRef());
    slang::ProgramLayout* layout = reflection->getLayout();
    if(!layout)
    {
        Console::warn("Failed to generate reflection data as program layout is invalid.", "Slang");
        return; // Deal with elsewhere
    }
    slang::EntryPointReflection* vertRefl = layout->getEntryPointByIndex(vertEntryPoint);
    slang::VariableLayoutReflection* globals = layout->getGlobalParamsVarLayout();
    if(globalLayout)
        reflectLayout(globals, globalLayout);
    if(vertLayout)
        reflectVertex(vertRefl, vertLayout);
}

void SlangReflect::reflectLayout(slang::VariableLayoutReflection* reflect, ShaderLayout* globalLayout)
{
    // Console::debugf("{}", reflect->get());
    // for(uint32_t i = 0; i < )
}

void SlangReflect::reflectVertex(slang::EntryPointReflection* reflect, VertexLayout* vertLayout)
{
    for(uint32_t i = 0; i < reflect->getParameterCount(); i++)
    {
        slang::VariableLayoutReflection* inputRefl = reflect->getParameterByIndex(i);
        slang::TypeLayoutReflection* typeLayout = inputRefl->getTypeLayout();
        for(uint32_t j = 0; j < typeLayout->getFieldCount(); j++)
        {
            slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(j);
            if(field->getSemanticName())
                Console::debugf("{}: {}", field->getName(), field->getSemanticName());
            else
                Console::debugf("{}", field->getName());
            Console::debugf("{}", field->getOffset(SLANG_PARAMETER_CATEGORY_VERTEX_INPUT));

            if(field->getSemanticName())
                std::string fieldSemantic = field->getSemanticName();
            // if(fieldSemantic == "POSITION")
            // {
                
            // }
        }
    }
}

}