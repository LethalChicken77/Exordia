#pragma once
#include <string>

namespace graphics
{
// Support any shader type
class Shader
{
public:
    enum ShaderType
    {
        VERTEX,
        FRAGMENT,
        COMPUTE,
        GEOMETRY,
        TESSELLATION_CONTROL,
        TESSELLATION_EVALUATION
    };
    std::string spirvCode;
};
} // namespace graphics