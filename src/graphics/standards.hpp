#pragma once
#include <string_view>

namespace graphics
{
enum class DescriptorBinding : uint16_t
{
    Camera = 0,
    Global = 1,
    Material = 2
};

// Shader strings
constexpr std::string_view VERTEX_INPUT_NAME = "VertexData";
constexpr std::string_view INSTANCE_INPUT_NAME = "InstanceData";

// Vertex layout
enum class VertBinding : uint16_t
{
    VertexPosition = 0,
    VertexBuffer = 1,
    InstanceBuffer = 2
};
}