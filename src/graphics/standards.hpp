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

// Vertex layout
constexpr unsigned int VERTEX_POSITION_BUFFER_BINDING = 0;
constexpr unsigned int VERTEX_BUFFER_BINDING = 1;
constexpr unsigned int INSTANCE_BUFFER_BINDING = 2;
}