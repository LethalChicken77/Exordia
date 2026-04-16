#include "shader_reflection.hpp"

#include <glm/glm.hpp>

#include "graphics/backend/vulkan_include.h"
#include "console.hpp"
#include "graphics/utils/alignment.hpp"

namespace graphics
{
using Attribute = VertexLayout::Attribute;
using AttrType = VertexLayout::AttrType;
using AttrSemantic = VertexLayout::AttrSemantic;
using AttrFormat = VertexLayout::AttrFormat;

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
    uint32_t bufferOffset = 0;
    reflect->getTypeLayout();
    // uint32_t weightsBufferoffset = 0;
    uint32_t locationBase = 0;
    for(uint32_t i = 0; i < reflect->getParameterCount(); i++)
    {
        // Leaving this as a loop in case I need to parse instance layout
        
        slang::VariableLayoutReflection* param = reflect->getParameterByIndex(i);
        slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
        // if (param->getCategory() == slang::ParameterCategory::VaryingInput ||
        //     param->getCategory() == slang::ParameterCategory::Mixed)
        uint32_t location;
        for(uint32_t j = 0; j < typeLayout->getFieldCount(); j++)
        {
            slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(j);
            // uint32_t location = field->getOffset(slang::ParameterCategory::VaryingInput);
            location = field->getBindingIndex() + locationBase;

            Attribute attribute{};
            // attribute.location = field->getOffset(SLANG_PARAMETER_CATEGORY_VERTEX_INPUT);
            attribute.location = (uint8_t)location;
            // attribute.location = field->getSemanticIndex();

            if(i == 1) 
            {
                Console::logf("{}", location);
                vertLayout->instanceBaseLocation = glm::min(vertLayout->instanceBaseLocation, (uint8_t)location);
                continue; // Don't add instance parameters to vertex layout
            }
            
            AttrFormat &attrFormat = attribute.format;
            
            const char* fieldSemantic_p = field->getSemanticName();
            if(fieldSemantic_p) // Yucky nesting, could handle in a helper
            {
                std::string fieldSemantic = fieldSemantic_p;

                if(fieldSemantic.starts_with("SV_"))
                    continue; // Ignore system semantics for layout

                if(fieldSemantic == "POSITION")
                    attribute.semantic = AttrSemantic::Position;
                else if(fieldSemantic == "NORMAL")
                    attribute.semantic = AttrSemantic::Normal;
                else if(fieldSemantic == "TANGENT")
                    attribute.semantic = AttrSemantic::Tangent;
                else if(fieldSemantic == "BITANGENT")
                    attribute.semantic = AttrSemantic::Bitangent;
                else if(fieldSemantic == "COLOR")
                    attribute.semantic = AttrSemantic::Color;
                else if(fieldSemantic.starts_with("UV") || fieldSemantic.starts_with("TEXCOORD"))
                    attribute.semantic = AttrSemantic::UV;
                else if(fieldSemantic == "MODEL")
                    attribute.semantic = AttrSemantic::I_Model;
            }

            attrFormat.componentCount = field->getType()->getColumnCount();
            attrFormat.arrayCount  = field->getType()->getRowCount();
            // uint32_t elementSize = 0;
            slang::TypeReflection::ScalarType scalarType = field->getType()->getScalarType();
            switch(scalarType)
            {
                case slang::TypeReflection::ScalarType::Int8:
                    attrFormat.type = AttrType::SInt;
                    attrFormat.componentSize = 1;
                    break;
                case slang::TypeReflection::ScalarType::Bool:
                case slang::TypeReflection::ScalarType::UInt8:
                    attrFormat.type = AttrType::UInt;
                    attrFormat.componentSize = 1;
                    break;
                case slang::TypeReflection::ScalarType::Int16:
                    attrFormat.type = AttrType::SInt;
                    attrFormat.componentSize = 2;
                    break;
                case slang::TypeReflection::ScalarType::UInt16:
                    attrFormat.type = AttrType::UInt;
                    attrFormat.componentSize = 2;
                    break;
                case slang::TypeReflection::ScalarType::Float16:
                    attrFormat.type = AttrType::Float;
                    attrFormat.componentSize = 2;
                    break;
                case slang::TypeReflection::ScalarType::Int32:
                    attrFormat.type = AttrType::SInt;
                    attrFormat.componentSize = 4;
                    break;
                case slang::TypeReflection::ScalarType::UInt32:
                    attrFormat.type = AttrType::UInt;
                    attrFormat.componentSize = 4;
                    break;
                case slang::TypeReflection::ScalarType::Float32:
                    attrFormat.type = AttrType::Float;
                    attrFormat.componentSize = 4;
                    break;
                default:
                    continue;
            }

            if(field->getVariable()->getUserAttributeCount() > 0)
            {
                if(attrFormat.type == AttrType::Float)
                {
                    std::string attrName = field->getVariable()->getUserAttributeByIndex(0)->getName();
                    if(attrName == "SNorm8")
                    {
                        attrFormat.type = AttrType::SNorm;
                        attrFormat.componentSize = 1;
                    }
                    else if(attrName == "UNorm8")
                    {
                        attrFormat.type = AttrType::UNorm;
                        attrFormat.componentSize = 1;
                    }
                    else if(attrName == "SNorm16")
                    {
                        attrFormat.type = AttrType::SNorm;
                        attrFormat.componentSize = 2;
                    }
                    else if(attrName == "UNorm16")
                    {
                        attrFormat.type = AttrType::UNorm;
                        attrFormat.componentSize = 2;
                    }
                }
            }

            if(attribute.semantic == AttrSemantic::Position)
            {
                attribute.offset = 0;
            }
            else
            {
                bufferOffset = Alignment::AlignUp(bufferOffset, attribute.format.GetAlignment(false)); // Don't worry about alignment
                attribute.offset = bufferOffset;
                bufferOffset += attribute.format.GetSize();
            }

            vertLayout->vertexAttributes.push_back(attribute);
        }
        if(location + 1 > locationBase) locationBase = location + 1;
    }
    Console::log(vertLayout->ToString());
}

}