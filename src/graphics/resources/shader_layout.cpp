#include "shader_layout.hpp"
#include "spirv_reflect.h"
#include "utils/console.hpp"

namespace graphics
{

DataType ParseType(const SpvReflectTypeDescription *td)
{

}

void ParseBlock(const SpvReflectBlockVariable *block, std::vector<ShaderParameter> *parameters)
{
    for(uint32_t m = 0; m < block->member_count; m++)
    {
        const SpvReflectBlockVariable member = block->members[m];
        
        std::string memberString = std::format("\t{}   \toffset = {} \tsize = {}", member.name, member.offset, member.size);
        memberString += std::format("\tType ID: {}", member.type_description->id);
        
        uint32_t bitWidth = member.type_description->traits.numeric.scalar.width;
        // uint32_t arrayLength = member.type_description->traits.numeric.vector.component_count;
        SpvOp op = member.type_description->op;
        uint32_t sign = member.type_description->traits.numeric.scalar.signedness;
        if(op == SpvOpTypeStruct)
        {
            ParseBlock(member.members, parameters);
            // result += 
            // Console::log(memberString);
            memberString += std::format("\tMember Count: {}", member.member_count);
            if(member.members[0].type_description->op == SpvOpTypeArray)
            {
                memberString += std::format(" \tArr Size: {}", member.members[0].array.dims[0]);
                memberString += std::format(" \tArr Type: {}", member.type_description->struct_type_description->id);
                memberString += std::format(" \tIs Vector: {}", member.members[0].type_description->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR);
                // ParseType(member.members[0].type_description);
            }
            Console::log(memberString);
        }
        else
        {
            DataType type = DataType::Invalid;
            parameters->emplace_back(ShaderParameter{
                member.name,
                member.offset,
            });
            if(op == SpvOpTypeInt)
            {
                if(sign)
                    type = DataType::Int;
                else
                    type = DataType::UInt;
            }
            else if(op == SpvOpTypeFloat)
            {
                type = DataType::Float;
            }
            else if(op == SpvOpTypeVector)
            {
                SpvReflectTypeDescription *td = member.type_description->struct_type_description;
                
            }
            if(op == SpvOpTypeArray)
            {
                memberString += std::format("Arr Size: {}", member.array.dims[0]);
            }
            Console::log(memberString);
        }
    }
}

ShaderLayout::ShaderLayout(std::vector<uint32_t> spirv, const std::string_view bufferName)
{
    Console::logf("Generating bindings for {}", bufferName, "ShaderAsset");
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
        if(binding && binding->name == bufferName)
        {
            foundMatInfo = true;
            Console::log(bufferName, "ShaderLayout", true);
            Console::logf("Descriptor Set  : {}", binding->set, "", false);
            Console::logf("Binding         : {}", binding->binding);
            Console::logf("Descriptor Type : {}", (uint32_t)binding->descriptor_type);
            Console::logf("Block size      : {}", binding->block.size);

            const SpvReflectBlockVariable* block = &binding->block;
            Console::logf("Fields ({}):", block->member_count);

            ParseBlock(block, &parameters);
        }
    }
}

std::string GetParameterString(ShaderParameter param)
{
    return std::format("\t{}: \tOffset: {} \tType: {} \tBase Size: {}\tCount: {}",
        param.name,
        param.offset,
        GetTypeString(param.type),
        param.typeSize,
        param.count);
}

} // namespace graphics