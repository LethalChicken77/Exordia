#pragma once
#include <glm/glm.hpp>
#include <string>
#include "graphics/backend/vulkan_include.h"

#include <iostream>

#ifdef DEBUG
    #if defined(_MSC_VER)
        #define DEBUG_BREAK() __debugbreak()
    #elif defined(__clang__) || defined(__GNUC__)
        #include <signal.h>
        #ifdef _POSIX
            #define DEBUG_BREAK() raise(SIGTRAP)
        #else
            #define DEBUG_BREAK() raise(SIGBREAK) // fallback, if applicable
        #endif
    #else
        #define DEBUG_BREAK() ((void)0)
    #endif
#else
    #define DEBUG_BREAK() ((void)0)
#endif

// Helper macros for stringification
#define STRINGIFY_HELPER(x) #x // turns x into a string
#define STRINGIFY(x) STRINGIFY_HELPER(x) // ensures proper expansion

// Macro for file:line
#define LOCATION_STR __FILE__ ":" STRINGIFY(__LINE__)
class Debug
{
    public:
        static std::string vec2ToString(const glm::vec2& v);
        static std::string vec3ToString(const glm::vec3& v);
        static std::string vec4ToString(const glm::vec4& v);

        static std::string mat2ToString(const glm::mat2& m);
        static std::string mat3ToString(const glm::mat3& m);
        static std::string mat4ToString(const glm::mat4& m);

        static std::string SlangStageToString(const uint32_t stage);

        static std::string VkResultToString(VkResult result);

        static void VkCheckImpl(VkResult result, const std::string& msg, const char* file, int line);
        static void VkCheckImplLight(VkResult result, const std::string& msg, const char* file, int line);
};

#ifndef DISABLE_VALIDATION
#define VK_CHECK(result, msg) Debug::VkCheckImpl(result, msg, __FILE__, __LINE__)
#else
#define VK_CHECK(result, msg) result
#endif