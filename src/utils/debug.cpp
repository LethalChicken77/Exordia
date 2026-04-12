#include "debug.hpp"
#include "console.hpp"
#include <iostream>
#include <sstream>
#include <string.h>
#include "graphics/backend/vulkan_include.h"
#include <slang.h>

std::string Debug::vec2ToString(const glm::vec2& v)
{
    std::ostringstream oss;
    oss << "vec2(" << v.x << ", " << v.y << ")";
    return oss.str();
}

std::string Debug::vec3ToString(const glm::vec3& v)
{
    std::ostringstream oss;
    oss << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return oss.str();
}

std::string Debug::vec4ToString(const glm::vec4& v)
{
    std::ostringstream oss;
    oss << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return oss.str();
}

std::string Debug::mat2ToString(const glm::mat2& m)
{
    std::ostringstream oss;
    oss << "mat2(\n";
    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 2; ++j)
        {
            oss << m[i][j];
            if (i != 1 || j != 1)
                oss << ", \t";
        }
        oss << "\n";
    }
    oss << ")\n";
    return oss.str();
}

std::string Debug::mat3ToString(const glm::mat3& m)
{
    std::ostringstream oss;
    oss << "mat3(\n";
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            oss << m[i][j];
            if (i != 1 || j != 1)
                oss << ", \t";
        }
        oss << "\n";
    }
    oss << ")\n";
    return oss.str();
}

std::string Debug::mat4ToString(const glm::mat4& m)
{
    std::ostringstream oss;
    oss << "mat4(\n";
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            oss << m[i][j];
            if (i != 1 || j != 1)
                oss << ", \t";
        }
        oss << "\n";
    }
    oss << ")\n";
    return oss.str();
}

std::string Debug::SlangStageToString(uint32_t _stage)
{
    SlangStage stage = (SlangStage)_stage;
    switch(stage)
    {
        case SLANG_STAGE_NONE:
            return "None";
        case SLANG_STAGE_VERTEX:
            return "Vertex";
        case SLANG_STAGE_HULL:
            return "Hull";
        case SLANG_STAGE_DOMAIN:
            return "Domain";
        case SLANG_STAGE_GEOMETRY:
            return "Geometry";
        case SLANG_STAGE_FRAGMENT:
            return "Fragment";
        case SLANG_STAGE_COMPUTE:
            return "Compute";
        case SLANG_STAGE_RAY_GENERATION:
            return "Ray Generation";
        case SLANG_STAGE_INTERSECTION:
            return "Intersection";
        case SLANG_STAGE_ANY_HIT:
            return "Any Hit";
        case SLANG_STAGE_CLOSEST_HIT:
            return "Closest Hit";
        case SLANG_STAGE_MISS:
            return "Miss";
        case SLANG_STAGE_CALLABLE:
            return "Callable";
        case SLANG_STAGE_MESH:
            return "Mesh";
        case SLANG_STAGE_AMPLIFICATION:
            return "Amplification";
        case SLANG_STAGE_COUNT:
            return "Count";
        default:
            return "";
    }
}

// Details obtained from https://docs.vulkan.org/refpages/latest/refpages/source/VkResult.html
std::string Debug::VkResultToString(VkResult result)
{
    // std::string base = "VkResult " + std::to_string(result) + ": ";
    std::string base = "";
    switch (result)
    {
        case VK_SUCCESS: return base + "Success: Command successfully completed";
        case VK_NOT_READY: return base + "Not ready: A fence or query has not yet completed";
        case VK_TIMEOUT: return base + "Timeout: A wait operation has not completed in the specified time";
        case VK_EVENT_SET: return base + "Event set: An event is signaled";
        case VK_EVENT_RESET: return base + "Event reset: An event is unsignaled";
        case VK_INCOMPLETE: return base + "Incomplete: A return array was too small for the result";
        case VK_SUBOPTIMAL_KHR: return base + "Suboptimal: A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully";
        case VK_THREAD_IDLE_KHR: return base + "Thread idle: A deferred operation is not complete but there is no work remaining to assign to a thread";
        case VK_THREAD_DONE_KHR: return base + "Thread done: A deferred operation is complete";
        case VK_OPERATION_DEFERRED_KHR: return base + "Operation deferred: A deferred operation has been scheduled";
        case VK_OPERATION_NOT_DEFERRED_KHR: return base + "Operation not deferred: A deferred operation was requested and no operations were deferred";
        case VK_PIPELINE_COMPILE_REQUIRED: return base + "Pipeline compile required: A requested pipeline creation would have required compilation, but the application requested compilation to not be performed";
        case VK_PIPELINE_BINARY_MISSING_KHR: return base + "Pipeline binary missing: The application attempted to create a pipeline binary by querying an internal cache, but the internal cache entry did not exist";
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT: return base + "Incompatible shader binary: The provided shader binary is not compatible with the current device";

        case VK_ERROR_OUT_OF_HOST_MEMORY: return base + "Out of host memory: A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return base + "Out of device memory: A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED: return base + "Initialization failed: Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST: return base + "Device lost: The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED: return base + "Memory map failed: Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT: return base + "Layer not present: A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return base + "Extension not present: A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT: return base + "Feature not present: A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return base + "Incompatible driver: The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons";
        case VK_ERROR_TOO_MANY_OBJECTS: return base + "Too many objects: Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return base + "Format not supported: A requested format is not supported on this device";
        case VK_ERROR_FRAGMENTED_POOL: return base + "Fragmented pool: A pool allocation has failed due to fragmentation of the pool's memory";

        case VK_ERROR_SURFACE_LOST_KHR: return base + "Surface lost: A surface is no longer available";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return base + "Native window in use: The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again";
        case VK_ERROR_OUT_OF_DATE_KHR: return base + "Out of date: A surface has changed in such a way that it is no longer compatible with the swapchain";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return base + "Incompatible display: The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image";
        case VK_ERROR_INVALID_SHADER_NV: return base + "Invalid shader: One or more shaders failed to compile or link";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return base + "Out of pool memory: A pool memory allocation has failed";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return base + "Invalid external handle: An external handle is not a valid handle of the specified type";
        case VK_ERROR_FRAGMENTATION: return base + "Fragmentation: A descriptor pool creation has failed due to fragmentation";
        case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT: return base + "Invalid device address: A buffer creation failed because the requested address is not available";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return base + "Full screen exclusive mode lost: An operation on a swapchain created with full-screen exclusive mode failed as the mode has been lost";
        case VK_ERROR_VALIDATION_FAILED_EXT: return base + "Validation failed: A validation layer found an error";
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return base + "Compression exhausted: A request to compress data has failed due to insufficient resources";
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return base + "Image usage not supported: The requested image usage is not supported for the specified image";
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return base + "Video picture layout not supported: The requested video picture layout is not supported for the specified video profile";
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return base + "Video profile operation not supported: The requested video profile operation is not supported";
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return base + "Video profile format not supported: The requested video profile format is not supported";
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return base + "Video profile codec not supported: The requested video profile codec is not supported";
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return base + "Video standard version not supported: The requested video standard header version is not supported";
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return base + "Invalid video standard parameters: The provided video standard parameters are not valid";
        case VK_ERROR_NOT_PERMITTED: return base + "Not permitted: The operation is not permitted";
        case VK_ERROR_NOT_ENOUGH_SPACE_KHR: return base + "Not enough space: There is not enough space left on the device to perform the operation";

        case VK_ERROR_UNKNOWN:
        default: return base + "Unknown error";
    }
}

// Details obtained from https://docs.vulkan.org/refpages/latest/refpages/source/VkResult.html
std::string Debug::VkResultToString(vk::Result result)
{
    // std::string base = "VkResult " + std::to_string(result) + ": ";
    std::string base = "";
    VkResult nativeResult = static_cast<VkResult>(result);
    switch (nativeResult)
    {
        case VK_SUCCESS: return base + "Success: Command successfully completed";
        case VK_NOT_READY: return base + "Not ready: A fence or query has not yet completed";
        case VK_TIMEOUT: return base + "Timeout: A wait operation has not completed in the specified time";
        case VK_EVENT_SET: return base + "Event set: An event is signaled";
        case VK_EVENT_RESET: return base + "Event reset: An event is unsignaled";
        case VK_INCOMPLETE: return base + "Incomplete: A return array was too small for the result";
        case VK_SUBOPTIMAL_KHR: return base + "Suboptimal: A swapchain no longer matches the surface properties exactly, but can still be used to present to the surface successfully";
        case VK_THREAD_IDLE_KHR: return base + "Thread idle: A deferred operation is not complete but there is no work remaining to assign to a thread";
        case VK_THREAD_DONE_KHR: return base + "Thread done: A deferred operation is complete";
        case VK_OPERATION_DEFERRED_KHR: return base + "Operation deferred: A deferred operation has been scheduled";
        case VK_OPERATION_NOT_DEFERRED_KHR: return base + "Operation not deferred: A deferred operation was requested and no operations were deferred";
        case VK_PIPELINE_COMPILE_REQUIRED: return base + "Pipeline compile required: A requested pipeline creation would have required compilation, but the application requested compilation to not be performed";
        case VK_PIPELINE_BINARY_MISSING_KHR: return base + "Pipeline binary missing: The application attempted to create a pipeline binary by querying an internal cache, but the internal cache entry did not exist";
        case VK_INCOMPATIBLE_SHADER_BINARY_EXT: return base + "Incompatible shader binary: The provided shader binary is not compatible with the current device";

        case VK_ERROR_OUT_OF_HOST_MEMORY: return base + "Out of host memory: A host memory allocation has failed";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return base + "Out of device memory: A device memory allocation has failed";
        case VK_ERROR_INITIALIZATION_FAILED: return base + "Initialization failed: Initialization of an object could not be completed for implementation-specific reasons";
        case VK_ERROR_DEVICE_LOST: return base + "Device lost: The logical or physical device has been lost";
        case VK_ERROR_MEMORY_MAP_FAILED: return base + "Memory map failed: Mapping of a memory object has failed";
        case VK_ERROR_LAYER_NOT_PRESENT: return base + "Layer not present: A requested layer is not present or could not be loaded";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return base + "Extension not present: A requested extension is not supported";
        case VK_ERROR_FEATURE_NOT_PRESENT: return base + "Feature not present: A requested feature is not supported";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return base + "Incompatible driver: The requested version of Vulkan is not supported by the driver or is otherwise incompatible for implementation-specific reasons";
        case VK_ERROR_TOO_MANY_OBJECTS: return base + "Too many objects: Too many objects of the type have already been created";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return base + "Format not supported: A requested format is not supported on this device";
        case VK_ERROR_FRAGMENTED_POOL: return base + "Fragmented pool: A pool allocation has failed due to fragmentation of the pool's memory";

        case VK_ERROR_SURFACE_LOST_KHR: return base + "Surface lost: A surface is no longer available";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return base + "Native window in use: The requested window is already in use by Vulkan or another API in a manner which prevents it from being used again";
        case VK_ERROR_OUT_OF_DATE_KHR: return base + "Out of date: A surface has changed in such a way that it is no longer compatible with the swapchain";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return base + "Incompatible display: The display used by a swapchain does not use the same presentable image layout, or is incompatible in a way that prevents sharing an image";
        case VK_ERROR_INVALID_SHADER_NV: return base + "Invalid shader: One or more shaders failed to compile or link";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return base + "Out of pool memory: A pool memory allocation has failed";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return base + "Invalid external handle: An external handle is not a valid handle of the specified type";
        case VK_ERROR_FRAGMENTATION: return base + "Fragmentation: A descriptor pool creation has failed due to fragmentation";
        case VK_ERROR_INVALID_DEVICE_ADDRESS_EXT: return base + "Invalid device address: A buffer creation failed because the requested address is not available";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return base + "Full screen exclusive mode lost: An operation on a swapchain created with full-screen exclusive mode failed as the mode has been lost";
        case VK_ERROR_VALIDATION_FAILED_EXT: return base + "Validation failed: A validation layer found an error";
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT: return base + "Compression exhausted: A request to compress data has failed due to insufficient resources";
        case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR: return base + "Image usage not supported: The requested image usage is not supported for the specified image";
        case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR: return base + "Video picture layout not supported: The requested video picture layout is not supported for the specified video profile";
        case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR: return base + "Video profile operation not supported: The requested video profile operation is not supported";
        case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR: return base + "Video profile format not supported: The requested video profile format is not supported";
        case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR: return base + "Video profile codec not supported: The requested video profile codec is not supported";
        case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR: return base + "Video standard version not supported: The requested video standard header version is not supported";
        case VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR: return base + "Invalid video standard parameters: The provided video standard parameters are not valid";
        case VK_ERROR_NOT_PERMITTED: return base + "Not permitted: The operation is not permitted";
        case VK_ERROR_NOT_ENOUGH_SPACE_KHR: return base + "Not enough space: There is not enough space left on the device to perform the operation";

        case VK_ERROR_UNKNOWN:
        default: return base + "Unknown error";
    }
}

/// @brief Checks the result of a vulkan error. Usually wrapped in the macro VK_CHECK
/// @param result Result returned from Vulkan API call
/// @param msg Base error message
/// @param file Set to __FILE__ by VK_CHECK
/// @param line Set to __LINE__ by VK_CHECK
void Debug::VkCheckImpl(VkResult result, const std::string& msg, const char* file, int line)
{
    if (result)
    {
        Console::error(std::format("Vulkan error {} at {}:{}", (int)result, file, line));
        std::cerr << msg << ": " << Debug::VkResultToString(result) << std::endl;
        throw std::runtime_error("Vulkan error - check log");
    }
}