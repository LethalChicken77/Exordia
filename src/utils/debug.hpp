#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vulkan/vulkan.h>

class Debug
{
    public:
        static std::string vec2ToString(const glm::vec2& v);
        static std::string vec3ToString(const glm::vec3& v);
        static std::string vec4ToString(const glm::vec4& v);

        static std::string mat2ToString(const glm::mat2& m);
        static std::string mat3ToString(const glm::mat3& m);
        static std::string mat4ToString(const glm::mat4& m);

        static std::string VkResultToString(VkResult result);
};