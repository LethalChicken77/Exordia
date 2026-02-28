#include "shader_layout.hpp"
#include "spirv_reflect.h"
#include "utils/console.hpp"

namespace graphics
{

ShaderLayout::ShaderLayout(std::vector<uint32_t> spirv, const std::string_view bufferName)
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
        if(binding && binding->name == bufferName)
        {
            foundMatInfo = true;
            Console::log(bufferName, "ShaderLayout", true);
            Console::logf("Descriptor Set  : {}", binding->set, "", false);
            Console::logf("Binding         : {}", binding->binding);
            Console::logf("Descriptor Type : {}", (uint32_t)binding->descriptor_type);
            Console::logf("Block size      : {}", binding->block.size);

            const SpvReflectBlockVariable& block = binding->block;
            Console::logf("Fields ({}):", block.member_count);

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

} // namespace graphics