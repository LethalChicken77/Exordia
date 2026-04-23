#include "shader_reflection.hpp"

#include <glm/glm.hpp>

#include "graphics/backend/vulkan_include.h"
#include "console.hpp"
#include "graphics/utils/alignment.hpp"

namespace graphics
{

void SlangReflect::GenerateLayouts(
    Slang::ComPtr<slang::ICompileRequest> request, 
    int vertEntryPoint, 
    int fragEntryPoint, 
    ShaderLayout* globalLayout, 
    VertexLayout* vertLayout, 
    uint32_t stageFlags)
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
        reflectLayout(globals, globalLayout, stageFlags);
    if(vertLayout)
        reflectVertex(vertRefl, vertLayout);
}

using DescriptorSetInfo = ShaderLayout::DescriptorSetInfo;
using PushConstantRange = ShaderLayout::PushConstantRange;
using BindingInfo = ShaderLayout::BindingInfo;
using BindingType = ShaderLayout::BindingType;
void SlangReflect::reflectLayout(slang::VariableLayoutReflection* reflect, ShaderLayout* globalLayout, uint32_t stageFlags)
{
    // TODO: Handle inline uniform blocks
    *globalLayout = ShaderLayout();
    slang::TypeLayoutReflection* typeLayout = reflect->getTypeLayout();
    std::unordered_map<uint32_t, DescriptorSetInfo*> setMap{};
    for(uint32_t i = 0; i < typeLayout->getFieldCount(); i++)
    {
        slang::VariableLayoutReflection* globalVar = typeLayout->getFieldByIndex(i);
// Console::debugf("{}", globalVar->getName());
        uint32_t set = globalVar->getBindingSpace();
        uint32_t binding = globalVar->getBindingIndex();
        DescriptorSetInfo *currentInfo = nullptr;

        BindingInfo bindingInfo{};
        bindingInfo.name = globalVar->getName();
        bindingInfo.set = currentInfo ? currentInfo->id : ~0u;
        bindingInfo.binding = binding;
        bindingInfo.stageFlags = stageFlags;
        
        slang::TypeReflection::Kind varKind = globalVar->getTypeLayout()->getKind();
        // Determine binding count
        bool isArray = varKind == slang::TypeReflection::Kind::Array;
        if(isArray)
        {
            bindingInfo.count = globalVar->getType()->getElementCount();
            // Handle array's subtype
            globalVar = globalVar->getTypeLayout()->getElementVarLayout();
            varKind = globalVar->getTypeLayout()->getKind();
        }
        else
            bindingInfo.count = 1;

        slang::ParameterCategory varCategory = globalVar->getTypeLayout()->getParameterCategory();
        SlangResourceAccess varAccess = globalVar->getType()->getResourceAccess();
        SlangResourceShape varShape = globalVar->getType()->getResourceShape();

        if(varCategory != slang::ParameterCategory::DescriptorTableSlot)
        {
            currentInfo = nullptr;
        }
        else if(!setMap.contains(set)) // Add new descriptor set
        {
            currentInfo = &globalLayout->descriptorSets.emplace_back(DescriptorSetInfo{
                set,
                {}
            });
            setMap.insert_or_assign(set, currentInfo);
        }
        else
        {
            currentInfo = setMap.at(set);
        }
        
        globalVar->getType()->getResourceResultType();
        slang::VariableLayoutReflection* fields = globalVar->getTypeLayout()->getElementVarLayout();
        switch(varKind)
        {
            case slang::TypeReflection::Kind::ConstantBuffer:
            {
                bindingInfo.type = BindingType::UniformBuffer;
                BufferLayout bufferLayout = reflectBuffer(fields);
                if(varCategory == slang::ParameterCategory::PushConstantBuffer)
                {
                    PushConstantRange pushConstants{};
                    pushConstants.layout = bufferLayout;
                    pushConstants.stageFlags = stageFlags;
                    globalLayout->pushConstantRanges.push_back(pushConstants);
                }
                else
                {
                    bindingInfo.bufferIndex = globalLayout->bufferLayouts.size();
                    if(bindingInfo.name == std::string("materialInfo"))
                    {
                        globalLayout->materialIndex = globalLayout->bufferLayouts.size();
                    }
                    globalLayout->bufferLayouts.push_back(bufferLayout);
                }
                break;
            }
            case slang::TypeReflection::Kind::ShaderStorageBuffer:
            {
                bindingInfo.type = BindingType::StorageBuffer;
                BufferLayout bufferLayout = reflectBuffer(fields);
                bindingInfo.bufferIndex = globalLayout->bufferLayouts.size();
                globalLayout->bufferLayouts.push_back(bufferLayout);
                break;
            }
            case slang::TypeReflection::Kind::Resource:
            {
                SlangResourceShape varShapeNoFlags = (SlangResourceShape)(varShape & ~SLANG_RESOURCE_EXT_SHAPE_MASK);
                SlangResourceShape varShapeFlags = (SlangResourceShape)(varShape & SLANG_RESOURCE_EXT_SHAPE_MASK);
                switch(varShapeNoFlags)
                {
                    case SLANG_STRUCTURED_BUFFER:
                    {
                        bindingInfo.type = BindingType::StorageBuffer;
                        BufferLayout bufferLayout = reflectBuffer(fields);
                        bindingInfo.bufferIndex = globalLayout->bufferLayouts.size();
                        globalLayout->bufferLayouts.push_back(bufferLayout);
                        break;
                    }
                    case SLANG_TEXTURE_1D:
                    {
                        if(varShapeFlags & SLANG_TEXTURE_COMBINED_FLAG)
                            bindingInfo.type = BindingType::CombinedImageSampler;
                        else
                            bindingInfo.type = BindingType::StorageImage;
                        TextureLayout textureLayout{};
                        textureLayout.shape = TextureShape::Cube;
                        bindingInfo.textureIndex = globalLayout->textureLayouts.size();
                        globalLayout->textureLayouts.push_back(textureLayout);
                        break;
                    }
                    case SLANG_TEXTURE_2D:
                    {
                        if(varShapeFlags & SLANG_TEXTURE_COMBINED_FLAG)
                            bindingInfo.type = BindingType::CombinedImageSampler;
                        else
                            bindingInfo.type = BindingType::StorageImage;
                        TextureLayout textureLayout{};
                        textureLayout.shape = TextureShape::Cube;
                        bindingInfo.textureIndex = globalLayout->textureLayouts.size();
                        globalLayout->textureLayouts.push_back(textureLayout);
                        break;
                    }
                    case SLANG_TEXTURE_3D:
                    {
                        if(varShapeFlags & SLANG_TEXTURE_COMBINED_FLAG)
                            bindingInfo.type = BindingType::CombinedImageSampler;
                        else
                            bindingInfo.type = BindingType::StorageImage;
                        TextureLayout textureLayout{};
                        textureLayout.shape = TextureShape::Cube;
                        bindingInfo.textureIndex = globalLayout->textureLayouts.size();
                        globalLayout->textureLayouts.push_back(textureLayout);
                        break;
                    }
                    case SLANG_TEXTURE_CUBE:
                    {
                        if(varShapeFlags & SLANG_TEXTURE_COMBINED_FLAG)
                            bindingInfo.type = BindingType::CombinedImageSampler;
                        else
                            bindingInfo.type = BindingType::StorageImage;
                        TextureLayout textureLayout{};
                        textureLayout.shape = TextureShape::Cube;
                        bindingInfo.textureIndex = globalLayout->textureLayouts.size();
                        globalLayout->textureLayouts.push_back(textureLayout);
                        break;
                    }
                    case SLANG_TEXTURE_BUFFER:
                    {
                        if(varAccess == SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ)
                            bindingInfo.type = BindingType::UniformTexelBuffer;
                        else if(varAccess == SlangResourceAccess::SLANG_RESOURCE_ACCESS_READ_WRITE)
                            bindingInfo.type = BindingType::StorageTexelBuffer;
                        
                        BufferLayout bufferLayout{};
                        ShaderParameter param{};
                        param.name = "";
                        param.type = reflectNumeric(globalVar->getTypeLayout()->getResourceResultType());
                        param.offset = 0;
                        bufferLayout.parameterIndex.insert_or_assign("", 0);
                        bufferLayout.parameters.push_back(param);
                        bindingInfo.bufferIndex = globalLayout->bufferLayouts.size();
                        globalLayout->bufferLayouts.push_back(bufferLayout);
                        break;
                    }
                    case SLANG_ACCELERATION_STRUCTURE:
                    {
                        bindingInfo.type = BindingType::AccelerationStructure;
                    }
                    default:
                        break; // Unhandled
                }
                break;
            }
            case slang::TypeReflection::Kind::SamplerState:
            {
                bindingInfo.type = BindingType::Sampler;
                TextureLayout textureLayout{};
// Console::debugf("Sampler shape: {}", (uint32_t)varShape);
                // textureLayout.shape = TextureShape::Cube;
                bindingInfo.textureIndex = globalLayout->textureLayouts.size();
                globalLayout->textureLayouts.push_back(textureLayout);
                break;
            }
            case slang::TypeReflection::Kind::DynamicResource: // TODO: Handle bindless resources
            default:
                Console::warnf("Unhandled global var kind: {}", (uint32_t)varKind);
                break;
        }
        if(currentInfo != nullptr)
        {

            currentInfo->bindings.push_back(bindingInfo);
        }
        
        // Console::debugf("\t{}", (int)field->getType()->getKind());
        // SlangImageFormat 
        // slang::TypeReflection::Kind;
        // slang::TypeReflection::Kind;
    }
}

BufferLayout SlangReflect::reflectBuffer(slang::VariableLayoutReflection* fields)
{
    BufferLayout layout{};
    std::vector<ShaderParameter> params = reflectStruct(fields, "", fields->getOffset());
    layout.totalSize = fields->getTypeLayout()->getSize();
    for(ShaderParameter& subparam : params)
    {
        layout.parameterIndex.insert_or_assign(subparam.name, layout.parameters.size());
        layout.parameters.push_back(subparam);
    }
// Console::logf("{}", layout.ToString());
    return layout;
}

TypeDescription SlangReflect::reflectNumeric(slang::TypeReflection* type)
{
    // slang::UserAttribute* attr = type->findUserAttributeByName("Range");
    TypeDescription td{};
    td.componentCount = type->getColumnCount();
    td.rowCount = type->getRowCount();
    slang::TypeReflection::ScalarType scalarType = type->getScalarType();
    switch(scalarType)
    {
        case slang::TypeReflection::ScalarType::Int8:
            td.type = DataType::SInt;
            td.componentSize = 1;
            break;
        case slang::TypeReflection::ScalarType::Bool:
            td.type = DataType::Bool;
            td.componentSize = 1;
            break;
        case slang::TypeReflection::ScalarType::UInt8:
            td.type = DataType::UInt;
            td.componentSize = 1;
            break;
        case slang::TypeReflection::ScalarType::Int16:
            td.type = DataType::SInt;
            td.componentSize = 2;
            break;
        case slang::TypeReflection::ScalarType::UInt16:
            td.type = DataType::UInt;
            td.componentSize = 2;
            break;
        case slang::TypeReflection::ScalarType::Float16:
            td.type = DataType::Float;
            td.componentSize = 2;
            break;
        case slang::TypeReflection::ScalarType::Int32:
            td.type = DataType::SInt;
            td.componentSize = 4;
            break;
        case slang::TypeReflection::ScalarType::UInt32:
            td.type = DataType::UInt;
            td.componentSize = 4;
            break;
        case slang::TypeReflection::ScalarType::Float32:
            td.type = DataType::Float;
            td.componentSize = 4;
            break;
        default:
            Console::warnf("Unrecognized scalar type: {}", (uint32_t)scalarType);
            td.type = DataType::Invalid;
            td.componentSize = 0;
            td.componentCount = 0;
            td.arrayCount = 0;
            break;
    }
    return td;
}

TypeDescription SlangReflect::reflectArray(slang::VariableLayoutReflection* field)
{
    TypeDescription td{};
    slang::TypeLayoutReflection* typeLayout = field->getTypeLayout();
    slang::TypeReflection::Kind arrKind = typeLayout->getElementTypeLayout()->getKind();
    switch(arrKind)
    {
    case slang::TypeReflection::Kind::Scalar:
    case slang::TypeReflection::Kind::Vector:
    case slang::TypeReflection::Kind::Matrix:
        td = reflectNumeric(typeLayout->getElementTypeLayout()->getType());
        break;
    case slang::TypeReflection::Kind::Array:
        td = reflectArray(typeLayout->getElementTypeLayout()->getFieldByIndex(0));
        break;
    case slang::TypeReflection::Kind::Struct:
        // td = reflectStruct(field);
        td.componentCount = 1;
        td.rowCount = 1;
        td.type = DataType::Struct;
        td.componentSize = typeLayout->getElementStride(SLANG_PARAMETER_CATEGORY_UNIFORM);
        break;
    default:
        Console::warnf("Unhandled array type: {}", (uint32_t)arrKind);
        break;
    }
    td.arrayCount = typeLayout->getElementCount();
    return td;
}

std::vector<ShaderParameter> SlangReflect::reflectStruct(slang::VariableLayoutReflection* fields, const std::string& parentName, uint32_t baseOffset)
{
    std::vector<ShaderParameter> params{};
    for(uint32_t j = 0; j < fields->getTypeLayout()->getFieldCount(); j++)
    {
        ShaderParameter param;
        slang::VariableLayoutReflection* field = fields->getTypeLayout()->getFieldByIndex(j);
        slang::TypeReflection::Kind fieldKind = field->getTypeLayout()->getKind();
        if(parentName != "")
            param.name = parentName + "." + field->getName();
        else
            param.name = field->getName();
        param.offset = field->getOffset() + baseOffset;
        TypeDescription& td = param.type;
        switch(fieldKind)
        {
        case slang::TypeReflection::Kind::Scalar:
        case slang::TypeReflection::Kind::Vector:
        case slang::TypeReflection::Kind::Matrix:
            param.type = reflectNumeric(field->getType());
            {
                uint32_t attributeCount = field->getVariable()->getUserAttributeCount();
                for(uint32_t i = 0; i < attributeCount; i++)
                {
                    slang::UserAttribute* attr = field->getVariable()->getUserAttributeByIndex(i);
                    std::string attrName = attr->getName();
                    if(attrName == "Range")
                    {
                        float min = 0;
                        float max = 0;
                        attr->getArgumentValueFloat(0, &min);
                        attr->getArgumentValueFloat(1, &max);
                        Console::debugf("Range attribute: {}, {}", min, max);
                    }
                }
            }
            break;
        case slang::TypeReflection::Kind::Array:
            param.type = reflectArray(field);
            break;
        case slang::TypeReflection::Kind::Struct:
            {
                std::vector<ShaderParameter> subparams = reflectStruct(field, param.name, param.offset);
                for(ShaderParameter& subparam : subparams)
                {
                    params.push_back(subparam);
                }
            }
            continue;
        default:
            Console::warnf("Unhandled field type: {}", (uint32_t)fieldKind);
            break;
        }
        params.push_back(param);
    }
    return params;
}

uint32_t SlangReflect::slangStageToVkStage(SlangStage stage)
{
    switch (stage)
    {
    case SLANG_STAGE_VERTEX:
        return VK_SHADER_STAGE_VERTEX_BIT;
    case SLANG_STAGE_FRAGMENT:
        return VK_SHADER_STAGE_FRAGMENT_BIT;
    case SLANG_STAGE_MESH:
        return VK_SHADER_STAGE_MESH_BIT_EXT;
    case SLANG_STAGE_COMPUTE:
        return VK_SHADER_STAGE_COMPUTE_BIT;
    default:
        return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    }
}

// ---------------------- Vertex Layout -------------------------

using Attribute = VertexLayout::Attribute;
using AttrSemantic = VertexLayout::AttrSemantic;

bool SlangReflect::handleVertexField(slang::VariableLayoutReflection* field, Attribute& attribute)
{    
    TypeDescription &attrFormat = attribute.format;
    
    const char* fieldSemantic_p = field->getSemanticName();
    if(fieldSemantic_p) // Yucky nesting, could handle in a helper
    {
        std::string fieldSemantic = fieldSemantic_p;

        if(fieldSemantic.starts_with("SV_"))
            return false; // Ignore system semantics for layout

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
            attrFormat.type = DataType::SInt;
            attrFormat.componentSize = 1;
            break;
        case slang::TypeReflection::ScalarType::Bool:
        case slang::TypeReflection::ScalarType::UInt8:
            attrFormat.type = DataType::UInt;
            attrFormat.componentSize = 1;
            break;
        case slang::TypeReflection::ScalarType::Int16:
            attrFormat.type = DataType::SInt;
            attrFormat.componentSize = 2;
            break;
        case slang::TypeReflection::ScalarType::UInt16:
            attrFormat.type = DataType::UInt;
            attrFormat.componentSize = 2;
            break;
        case slang::TypeReflection::ScalarType::Float16:
            Console::warn("16 bit floats are not supported. Use \"[Half] float\" instead.");
            return false;
        case slang::TypeReflection::ScalarType::Int32:
            attrFormat.type = DataType::SInt;
            attrFormat.componentSize = 4;
            break;
        case slang::TypeReflection::ScalarType::UInt32:
            attrFormat.type = DataType::UInt;
            attrFormat.componentSize = 4;
            break;
        case slang::TypeReflection::ScalarType::Float32:
            attrFormat.type = DataType::Float;
            attrFormat.componentSize = 4;
            break;
        default:
            return false;
    }

    if(field->getVariable()->getUserAttributeCount() > 0)
    {
        if(attrFormat.type == DataType::Float)
        {
            std::string attrName = field->getVariable()->getUserAttributeByIndex(0)->getName();
            if(attrName == "SNorm8")
            {
                attrFormat.type = DataType::SNorm;
                attrFormat.componentSize = 1;
            }
            else if(attrName == "UNorm8")
            {
                attrFormat.type = DataType::UNorm;
                attrFormat.componentSize = 1;
            }
            else if(attrName == "SNorm16")
            {
                attrFormat.type = DataType::SNorm;
                attrFormat.componentSize = 2;
            }
            else if(attrName == "UNorm16")
            {
                attrFormat.type = DataType::UNorm;
                attrFormat.componentSize = 2;
            }
            else if(attrName == "Half")
            {
                attrFormat.type = DataType::Float;
                attrFormat.componentSize = 2;
            }
        }
    }
    return true;
}

void SlangReflect::reflectVertex(slang::EntryPointReflection* reflect, VertexLayout* vertLayout)
{
    reflect->getTypeLayout();
    // uint32_t weightsBufferoffset = 0;
    uint32_t locationBase = 0;
    *vertLayout = VertexLayout();
    for(uint32_t i = 0; i < reflect->getParameterCount(); i++)
    {
        // Leaving this as a loop in case I need to parse instance layout
        
        slang::VariableLayoutReflection* param = reflect->getParameterByIndex(i);
        slang::TypeLayoutReflection* typeLayout = param->getTypeLayout();
        // if (param->getCategory() == slang::ParameterCategory::VaryingInput ||
        //     param->getCategory() == slang::ParameterCategory::Mixed)
        uint32_t nextLocation;
        uint32_t bufferOffset = 0;
        for(uint32_t j = 0; j < typeLayout->getFieldCount(); j++)
        {
            slang::VariableLayoutReflection* field = typeLayout->getFieldByIndex(j);
            
            Attribute attribute{};
            bool result = handleVertexField(field, attribute);
            uint8_t location = param->getOffset(SLANG_PARAMETER_CATEGORY_VARYING_INPUT) + field->getOffset(SLANG_PARAMETER_CATEGORY_VARYING_INPUT);
            
            // nextLocation = glm::max(localLocation + locationBase + attribute.format.arrayCount, nextLocation);
            
            // attribute.location = field->getOffset(SlangParameterCategory);
            // Console::logf("Name: {}", field->getName());
            // Console::logf("\tLocation: {}", location);
            if(i == 1) 
            {
                if(attribute.semantic == AttrSemantic::I_Model)
                    vertLayout->instanceBaseLocation = glm::min(vertLayout->instanceBaseLocation, location);
                break; // Don't add instance parameters to vertex layout
            }
            // attribute.location = field->getOffset(SLANG_PARAMETER_CATEGORY_VERTEX_INPUT);
            attribute.location = location;
            // attribute.location = field->getSemanticIndex();
            
            if(!result) continue;

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
    }
    // Console::logf("{}", vertLayout->ToString());
}



// ---------------------------------SPIRV REFLECT--------------------------------------


// BufferLayout::BufferLayout(const SpvReflectBlockVariable* block)
// {
//     // Console::logf("Generating bindings for {}", block->name, "ShaderAsset");
//     totalSize = block->size;
//     ParseBlock(block, &parameters);
//     for(uint32_t i = 0; i < parameters.size(); ++i)
//     {
//         const ShaderParameter &param = parameters[i];
//         parameterIndex.insert_or_assign(param.name, &param);
//         Console::log(param.ToString(), "BufferLayout");
//     }
// }

// ShaderLayout::ShaderLayout(const std::vector<uint32_t> spirv)
// {
//     spv_reflect::ShaderModule smodule(spirv);

//     uint32_t bindingCount = 0;
//     SpvReflectResult result = smodule.EnumerateDescriptorBindings(&bindingCount, nullptr);
//     if(result != SPV_REFLECT_RESULT_SUCCESS)
//     {
//         Console::error(std::format("Failed to enumerate descriptor bindings: {}", (uint32_t)result), "ShaderAsset");
//         return;
//     }
    
//     if(bindingCount == 0)
//     {
//         Console::log("No descriptor bindings found.", "ShaderAsset");
//         return;
//     }

//     std::vector<SpvReflectDescriptorBinding*> bindings(bindingCount);
//     result = smodule.EnumerateDescriptorBindings(&bindingCount, bindings.data());
//     if(result != SPV_REFLECT_RESULT_SUCCESS)
//     {
//         Console::error(std::format("Failed to retrieve descriptor bindings: {}", (uint32_t)result), "ShaderAsset");
//         return;
//     }

//     std::unordered_map<uint32_t, DescriptorSetInfo*> setMap{};
//     for(SpvReflectDescriptorBinding* binding : bindings)
//     {
//         DescriptorSetInfo *currentInfo = nullptr;
//         if(!setMap.contains(binding->set)) // Add new descriptor set
//         {
//             currentInfo = &descriptorSets.emplace_back(DescriptorSetInfo{
//                 binding->set,
//                 {}
//             });
//             setMap.insert_or_assign(binding->set, currentInfo);
//         }
//         else
//         {
//             currentInfo = setMap.at(binding->set);
//         }
//         BindingInfo bindingInfo{};
//         bindingInfo.name = binding->name;
//         bindingInfo.count = binding->count;
//         bindingInfo.set = binding->set;
//         bindingInfo.stageFlags = smodule.GetShaderStage(); // TODO: Modify to ensure all relevant stages are included
//         bindingInfo.binding = binding->binding;
//         Console::logf("{} \tSet: {}, Binding: {}", bindingInfo.name, bindingInfo.set, bindingInfo.binding);
//         switch(binding->descriptor_type)
//         {
//             case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
//                 bindingInfo.type = BindingType::Sampler;
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
//                 bindingInfo.type = BindingType::CombinedImageSampler;
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
//                 bindingInfo.type = BindingType::SampledImage;
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
//                 bindingInfo.type = BindingType::StorageImage;
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
//                 bindingInfo.type = BindingType::InputAttachment;
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
//                 bindingInfo.type = BindingType::UniformTexelBuffer;
//                 // TODO: Figure out how to correctly handle texel buffers
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
//                 bindingInfo.type = BindingType::StorageTexelBuffer;
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
//                 bindingInfo.type = BindingType::UniformBuffer;
//                 generateBufferInfo(binding, &bindingInfo);
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
//                 bindingInfo.type = BindingType::StorageBuffer;
//                 generateBufferInfo(binding, &bindingInfo);
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
//                 bindingInfo.type = BindingType::DynamicUniformBuffer;
//                 generateBufferInfo(binding, &bindingInfo);
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
//                 bindingInfo.type = BindingType::DynamicStorageBuffer;
//                 generateBufferInfo(binding, &bindingInfo);
//                 break;
//             case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
//                 bindingInfo.type = BindingType::AccelerationStructure;
//                 break;
//         }

//         currentInfo->bindings.push_back(bindingInfo);
//     }

// }

// void ShaderLayout::generateBufferInfo(const SpvReflectDescriptorBinding *binding, BindingInfo* info)
// {
//     const SpvReflectBlockVariable *block = &binding->block;
//     info->bufferIndex = bufferLayouts.size();
//     bufferLayouts.emplace_back(BufferLayout(block));
//     if(block->name == std::string("cameraData"))
//     {
//         cameraInfo = &bufferLayouts.back();
//     }
//     else if(block->name == std::string("globalData"))
//     {
//         globalInfo = &bufferLayouts.back();
//     }
//     else if(block->name == std::string("materialInfo"))
//     {
//         materialInfo = &bufferLayouts.back();
//     }
// }

// inline ShaderParameter CreateBoolParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
// {
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         DataType::Bool,
//         member.type_description->traits.numeric.scalar.width / 8,
//         member.size,
//         1,
//         false
//     );
// }

// inline ShaderParameter CreateIntParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
// {
//     DataType type;
//     if(member.type_description->traits.numeric.scalar.signedness)
//         type = DataType::SInt;
//     else
//         type = DataType::UInt;
    
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         member.type_description->traits.numeric.scalar.width / 8,
//         member.size,
//         1,
//         false
//     );
// }

// inline ShaderParameter CreateFloatParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         DataType::Float,
//         member.type_description->traits.numeric.scalar.width / 8,
//         member.size,
//         1,
//         false
//     );
// }

// inline ShaderParameter CreateVectorIntParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     DataType type;
//     const SpvReflectTypeDescription *td = member.type_description;
//     if(td->traits.numeric.vector.component_count == 2)
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec2;
//         else
//             type = DataType::UVec2;
//     else if(td->traits.numeric.vector.component_count == 3)
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec3;
//         else
//             type = DataType::UVec3;
//     else
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec4;
//         else
//             type = DataType::UVec4;
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         td->traits.numeric.scalar.width / 8,
//         member.size,
//         1,
//         false
//     );
// }

// inline ShaderParameter CreateVectorFloatParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     DataType type;
//     const SpvReflectTypeDescription *td = member.type_description;
//     if(td->traits.numeric.vector.component_count == 2)
//         type = DataType::Vec2;
//     else if(td->traits.numeric.vector.component_count == 3)
//         type = DataType::Vec3;
//     else
//         type = DataType::Vec4;

//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         td->traits.numeric.scalar.width / 8,
//         member.size,
//         1,
//         false
//     );
// }

// inline ShaderParameter CreateMatrixIntParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     DataType type;
//     const SpvReflectTypeDescription *td = member.type_description;
//     if(td->traits.numeric.vector.component_count == 2)
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec2;
//         else
//             type = DataType::UVec2;
//     else if(td->traits.numeric.vector.component_count == 3)
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec3;
//         else
//             type = DataType::UVec3;
//     else
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec4;
//         else
//             type = DataType::UVec4;
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         td->traits.numeric.scalar.width / 8,
//         member.size,
//         1,
//         false
//     );
// }

// inline ShaderParameter CreateMatrixFloatParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     DataType type;
//     const SpvReflectTypeDescription *td = member.type_description;
//     if(td->traits.numeric.matrix.row_count == 2)
//         type = DataType::Vec2;
//     else if(td->traits.numeric.matrix.row_count == 3)
//         type = DataType::Vec3;
//     else
//         type = DataType::Vec4;

//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         td->traits.numeric.scalar.width / 8,
//         member.size,
//         td->traits.numeric.matrix.column_count,
//         false
//     );
// }

// inline ShaderParameter CreateBoolParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
// {
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         DataType::Bool,
//         member.type_description->traits.numeric.scalar.width / 8,
//         member.array.stride,
//         member.array.dims[0], // TODO: Support multidimensional arrays
//         false
//     );
// }

// inline ShaderParameter CreateIntParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
// {
//     DataType type;
//     if(member.type_description->traits.numeric.scalar.signedness)
//         type = DataType::Int;
//     else
//         type = DataType::UInt;
    
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         member.type_description->traits.numeric.scalar.width / 8,
//         member.array.stride,
//         member.array.dims[0], // TODO: Support multidimensional arrays
//         false
//     );
// }

// inline ShaderParameter CreateFloatParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         DataType::Float,
//         member.type_description->traits.numeric.scalar.width / 8,
//         member.array.stride,
//         member.array.dims[0], // TODO: Support multidimensional arrays
//         false
//     );
// }

// inline ShaderParameter CreateVectorIntParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     DataType type;
//     const SpvReflectTypeDescription *td = member.type_description;
//     if(td->traits.numeric.vector.component_count == 2)
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec2;
//         else
//             type = DataType::UVec2;
//     else if(td->traits.numeric.vector.component_count == 3)
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec3;
//         else
//             type = DataType::UVec3;
//     else
//         if(td->traits.numeric.scalar.signedness)
//             type = DataType::IVec4;
//         else
//             type = DataType::UVec4;
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         td->traits.numeric.scalar.width / 8,
//         member.array.stride,
//         member.array.dims[0], // TODO: Support multidimensional arrays
//         false
//     );
// }

// inline ShaderParameter CreateVectorFloatParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     DataType type;
//     const SpvReflectTypeDescription *td = member.type_description;
//     if(td->traits.numeric.vector.component_count == 2)
//         type = DataType::Vec2;
//     else if(td->traits.numeric.vector.component_count == 3)
//         type = DataType::Vec3;
//     else
//         type = DataType::Vec4;

//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         type,
//         td->traits.numeric.scalar.width / 8,
//         member.array.stride,
//         member.array.dims[0], // TODO: Support multidimensional arrays
//         false
//     );
// }

// inline ShaderParameter CreateStructArray(const SpvReflectBlockVariable &member, std::string parentName = "")
// {    
//     return ShaderParameter(
//         parentName + member.name,
//         member.absolute_offset,
//         DataType::Struct,
//         0,
//         member.array.stride,
//         member.array.dims[0], // TODO: Support multidimensional arrays
//         false
//     );
// }


// void ParseBlock(const SpvReflectBlockVariable *block, std::vector<ShaderParameter> *parameters, std::string parentName = "");

// void ParseMember(const SpvReflectBlockVariable &member, std::vector<ShaderParameter> *parameters, std::string parentName = "")
// {
//     const SpvReflectTypeDescription *td = member.type_description;
//     SpvOp op = td->op;
//     if(op == SpvOpTypeStruct)
//     {
//         ParseBlock(&member, parameters, parentName + member.name + ".");
//     }
//     else if(op == SpvOpTypeArray)
//     {
//         if((td->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) && (td->type_flags & SPV_REFLECT_TYPE_FLAG_INT))
//         {
//             parameters->emplace_back(CreateVectorIntParameterArray(member, parentName.substr(0, parentName.length() - 1)));
//         }
//         else if((td->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) && (td->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT))
//         {
//             parameters->emplace_back(CreateVectorFloatParameterArray(member, parentName.substr(0, parentName.length() - 1)));
//         }
//         else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
//         {
//             parameters->emplace_back(CreateBoolParameterArray(member, parentName.substr(0, parentName.length() - 1)));
//         }
//         else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
//         {
//             parameters->emplace_back(CreateIntParameterArray(member, parentName.substr(0, parentName.length() - 1)));
//         }
//         else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
//         {
//             parameters->emplace_back(CreateIntParameterArray(member, parentName.substr(0, parentName.length() - 1)));
//         }
//         else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT)
//         {
//             parameters->emplace_back(CreateStructArray(member, parentName.substr(0, parentName.length() - 1)));
//         }
//         else
//         {
//             Console::logf("Unhandled array type: {:b}", (uint32_t)td->type_flags, "BufferLayout");
//         }
//     }
//     else if(op == SpvOpTypeInt)
//     {
//         parameters->emplace_back(CreateIntParameter(member, parentName));
//     }
//     else if(op == SpvOpTypeFloat)
//     {
//         parameters->emplace_back(CreateFloatParameter(member, parentName));
//     }
//     else if(op == SpvOpTypeBool)
//     {
//         parameters->emplace_back(CreateBoolParameter(member, parentName));
//     }
//     else if(op == SpvOpTypeVector)
//     {
//         if(td->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
//             parameters->emplace_back(CreateVectorIntParameter(member, parentName));
//         else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
//             parameters->emplace_back(CreateVectorFloatParameter(member, parentName));        
//     }
//     else if(op == SpvOpTypeMatrix)
//     {
//         if(td->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
//             parameters->emplace_back(CreateMatrixIntParameter(member, parentName));
//         else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
//             parameters->emplace_back(CreateMatrixFloatParameter(member, parentName));        
//     }
//     else
//     {
//         Console::logf("Unhandled op type: {}", (uint32_t)op, "BufferLayout");
//     }
// }

// void ParseBlock(const SpvReflectBlockVariable *block, std::vector<ShaderParameter> *parameters, std::string parentName)
// {
//     for(uint32_t m = 0; m < block->member_count; m++)
//     {
//         const SpvReflectBlockVariable &member = block->members[m];
//         ParseMember(member, parameters, parentName);
//     }
// }

}