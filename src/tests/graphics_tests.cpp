#ifdef GRAPHICS_TESTS
#include <format>
#include <memory>
#include "graphics_tests.hpp"
#include "graphics/graphics_data.hpp"
#include "backend/buffer.hpp"
#include "utils/console.hpp"
#include "utils/debug.hpp"
namespace graphics::tests
{
    std::unique_ptr<Buffer> testBuffer;
    bool TestBufferCreation()
    {
        Console::log("Running Buffer Creation Test", "GraphicsTests");
        constexpr VkDeviceSize instanceSize = 64;
        constexpr uint32_t instanceCount = 10;
        constexpr VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        constexpr VkMemoryPropertyFlags memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        try
        {
            testBuffer = std::make_unique<Buffer>(
                instanceSize,
                instanceCount,
                usageFlags,
                memoryPropertyFlags
            );
            Console::log("Buffer Creation Test Passed", "GraphicsTests");
            return true;
        }
        catch(const std::exception& e)
        {
            Console::error(std::format("Buffer Creation Test Failed: {}", e.what()), "GraphicsTests");
            return false;
        }
    }

    bool TestBufferMapping()
    {
        Console::log("Running Buffer Mapping Test", "GraphicsTests");
        if(!testBuffer)
        {
            Console::error("Buffer Mapping Test Failed: Test buffer not created", "GraphicsTests");
            return false;
        }

        VkResult result = testBuffer->Map();
        if(result != VK_SUCCESS)
        {
            Console::error(std::format("Buffer Mapping Test Failed: {}", Debug::VkResultToString(result)), "GraphicsTests");
            return false;
        }

        testBuffer->Unmap();
        Console::log("Buffer Mapping Test Passed", "GraphicsTests");
        return true;
    }

    void RunAllTests()
    {
        TestBufferCreation();
        TestBufferMapping();
    }
}

#endif