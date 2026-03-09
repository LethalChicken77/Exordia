#pragma once
#include "core/resources/material.hpp"
#include "graphics/backend/buffer.hpp"
#include "graphics_pipeline.hpp"

namespace graphics
{

class ShaderBuffer // Graphics version of a material
{
public:
    ShaderBuffer(const core::Material *material);
private:
    const core::Material *material;
    Buffer buffer;
};

}