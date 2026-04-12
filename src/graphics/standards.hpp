#pragma once
#include <string_view>

namespace graphics
{
constexpr unsigned int CAMERA_DESCRIPTOR_SET = 0;
constexpr unsigned int GLOBAL_DESCRIPTOR_SET = 1;
constexpr unsigned int MATERIAL_DESCRIPTOR_SET = 2;

// Shader strings
constexpr std::string_view VERTEX_INPUT_NAME = "VertexData";
constexpr std::string_view INSTANCE_INPUT_NAME = "InstanceData";
}