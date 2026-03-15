#include "shader_layout.hpp"
#include "spirv_reflect.h"
#include "utils/console.hpp"

namespace graphics
{

inline ShaderParameter CreateBoolParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
{
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        DataType::Bool,
        member.type_description->traits.numeric.scalar.width / 8,
        member.size,
        1,
        false
    );
}

inline ShaderParameter CreateIntParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
{
    DataType type;
    if(member.type_description->traits.numeric.scalar.signedness)
        type = DataType::Int;
    else
        type = DataType::UInt;
    
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        type,
        member.type_description->traits.numeric.scalar.width / 8,
        member.size,
        1,
        false
    );
}

inline ShaderParameter CreateFloatParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
{    
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        DataType::Float,
        member.type_description->traits.numeric.scalar.width / 8,
        member.size,
        1,
        false
    );
}

inline ShaderParameter CreateVectorIntParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
{    
    DataType type;
    const SpvReflectTypeDescription *td = member.type_description;
    if(td->traits.numeric.vector.component_count == 2)
        if(td->traits.numeric.scalar.signedness)
            type = DataType::IVec2;
        else
            type = DataType::UVec2;
    else if(td->traits.numeric.vector.component_count == 3)
        if(td->traits.numeric.scalar.signedness)
            type = DataType::IVec3;
        else
            type = DataType::UVec3;
    else
        if(td->traits.numeric.scalar.signedness)
            type = DataType::IVec4;
        else
            type = DataType::UVec4;
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        type,
        td->traits.numeric.scalar.width / 8,
        member.size,
        1,
        false
    );
}

inline ShaderParameter CreateVectorFloatParameter(const SpvReflectBlockVariable &member, std::string parentName = "")
{    
    DataType type;
    const SpvReflectTypeDescription *td = member.type_description;
    if(td->traits.numeric.vector.component_count == 2)
        type = DataType::Vec2;
    else if(td->traits.numeric.vector.component_count == 3)
        type = DataType::Vec3;
    else
        type = DataType::Vec4;

    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        type,
        td->traits.numeric.scalar.width / 8,
        member.size,
        1,
        false
    );
}

inline ShaderParameter CreateBoolParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
{
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        DataType::Bool,
        member.type_description->traits.numeric.scalar.width / 8,
        member.array.stride,
        member.array.dims[0], // TODO: Support multidimensional arrays
        false
    );
}

inline ShaderParameter CreateIntParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
{
    DataType type;
    if(member.type_description->traits.numeric.scalar.signedness)
        type = DataType::Int;
    else
        type = DataType::UInt;
    
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        type,
        member.type_description->traits.numeric.scalar.width / 8,
        member.array.stride,
        member.array.dims[0], // TODO: Support multidimensional arrays
        false
    );
}

inline ShaderParameter CreateFloatParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
{    
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        DataType::Float,
        member.type_description->traits.numeric.scalar.width / 8,
        member.array.stride,
        member.array.dims[0], // TODO: Support multidimensional arrays
        false
    );
}

inline ShaderParameter CreateVectorIntParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
{    
    DataType type;
    const SpvReflectTypeDescription *td = member.type_description;
    if(td->traits.numeric.vector.component_count == 2)
        if(td->traits.numeric.scalar.signedness)
            type = DataType::IVec2;
        else
            type = DataType::UVec2;
    else if(td->traits.numeric.vector.component_count == 3)
        if(td->traits.numeric.scalar.signedness)
            type = DataType::IVec3;
        else
            type = DataType::UVec3;
    else
        if(td->traits.numeric.scalar.signedness)
            type = DataType::IVec4;
        else
            type = DataType::UVec4;
    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        type,
        td->traits.numeric.scalar.width / 8,
        member.array.stride,
        member.array.dims[0], // TODO: Support multidimensional arrays
        false
    );
}

inline ShaderParameter CreateVectorFloatParameterArray(const SpvReflectBlockVariable &member, std::string parentName = "")
{    
    DataType type;
    const SpvReflectTypeDescription *td = member.type_description;
    if(td->traits.numeric.vector.component_count == 2)
        type = DataType::Vec2;
    else if(td->traits.numeric.vector.component_count == 3)
        type = DataType::Vec3;
    else
        type = DataType::Vec4;

    return ShaderParameter(
        parentName + member.name,
        member.absolute_offset,
        type,
        td->traits.numeric.scalar.width / 8,
        member.array.stride,
        member.array.dims[0], // TODO: Support multidimensional arrays
        false
    );
}


void ParseBlock(const SpvReflectBlockVariable *block, std::vector<ShaderParameter> *parameters, std::string parentName = "");

void ParseMember(const SpvReflectBlockVariable &member, std::vector<ShaderParameter> *parameters, std::string parentName = "")
{
    const SpvReflectTypeDescription *td = member.type_description;
    SpvOp op = td->op;
    if(op == SpvOpTypeStruct)
    {
        ParseBlock(&member, parameters, parentName + member.name + ".");
    }
    else if(op == SpvOpTypeArray)
    {
        if((td->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) && (td->type_flags & SPV_REFLECT_TYPE_FLAG_INT))
        {
            parameters->emplace_back(CreateVectorIntParameterArray(member, parentName.substr(0, parentName.length() - 1)));
        }
        else if((td->type_flags & SPV_REFLECT_TYPE_FLAG_VECTOR) && (td->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT))
        {
            parameters->emplace_back(CreateVectorFloatParameterArray(member, parentName.substr(0, parentName.length() - 1)));
        }
        else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_BOOL)
        {
            parameters->emplace_back(CreateBoolParameterArray(member, parentName.substr(0, parentName.length() - 1)));
        }
        else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
        {
            parameters->emplace_back(CreateIntParameterArray(member, parentName.substr(0, parentName.length() - 1)));
        }
        else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
        {
            parameters->emplace_back(CreateIntParameterArray(member, parentName.substr(0, parentName.length() - 1)));
        }
        else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_STRUCT)
        {
            Console::log("Arrays of structs not yet supported", "BufferLayout"); // TODO: Support arrays of structs
        }
        else
        {
            Console::logf("Unhandled array type: {:b}", (uint32_t)td->type_flags, "BufferLayout");
        }
    }
    else if(op == SpvOpTypeInt)
    {
        parameters->emplace_back(CreateIntParameter(member, parentName));
    }
    else if(op == SpvOpTypeFloat)
    {
        parameters->emplace_back(CreateFloatParameter(member, parentName));
    }
    else if(op == SpvOpTypeBool)
    {
        parameters->emplace_back(CreateBoolParameter(member, parentName));
    }
    else if(op == SpvOpTypeVector)
    {
        if(td->type_flags & SPV_REFLECT_TYPE_FLAG_INT)
            parameters->emplace_back(CreateVectorIntParameter(member, parentName));
        else if(td->type_flags & SPV_REFLECT_TYPE_FLAG_FLOAT)
            parameters->emplace_back(CreateVectorFloatParameter(member, parentName));        
    }
    else
    {
        Console::logf("Unhandled op type: {}", (uint32_t)op, "BufferLayout");
    }
}

void ParseBlock(const SpvReflectBlockVariable *block, std::vector<ShaderParameter> *parameters, std::string parentName)
{
    for(uint32_t m = 0; m < block->member_count; m++)
    {
        const SpvReflectBlockVariable &member = block->members[m];
        ParseMember(member, parameters, parentName);
    }
}

BufferLayout::BufferLayout(std::vector<uint32_t> spirv, const std::string_view bufferName)
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
            const SpvReflectBlockVariable* block = &binding->block;
            ParseBlock(block, &parameters);
            totalSize = block->size;
            break;
        }
    }
    for(uint32_t i = 0; i < parameters.size(); ++i)
    {
        const ShaderParameter &param = parameters[i];
        parameterIndex.insert_or_assign(param.name, &param);
        Console::log(GetParameterString(param), "BufferLayout");
    }
}

std::string GetParameterString(ShaderParameter param)
{
    return std::format("{:>15} | Offset: {:>5} | Type: {:>6} | Base Size: {:>2} | Stride: {:>4} | Count: {:>6}",
        param.name,
        param.offset,
        GetTypeString(param.type),
        param.baseSize,
        param.size,
        param.count);
}

} // namespace graphics