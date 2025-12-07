#ifdef GRAPHICS_TESTS
#include <format>
#include <memory>
#include "graphics_tests.hpp"
#include "graphics/graphics_data.hpp"
#include "graphics/backend/buffer.hpp"
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

    void BasicBufferTest()
    {
        testBuffer = std::make_unique<Buffer>(
            sizeof(float),
            4,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        Console::log("Test buffer created", "Graphics");
        testBuffer->Map();
        Console::log("Test buffer mapped", "Graphics");

        float floatData[4]{0.0f, 1.0f, 2.0f, 3.0f};
        testBuffer->WriteData(static_cast<void*>(&floatData), sizeof(float) * 4);

        float resultData[4]{-1.0f, -1.0f, -1.0f, -1.0f};
        testBuffer->ReadData(static_cast<void*>(&resultData), sizeof(float) * 4);
        Console::log(std::format("Read back data from test buffer: {}, {}, {}, {}", resultData[0], resultData[1], resultData[2], resultData[3]), "Graphics");
        
        float newData[2] {6.0f, 7.0f};
        testBuffer->WriteToIndex(static_cast<void*>(&newData), 1, 2);
        testBuffer->ReadFromIndex(static_cast<void*>(&resultData), 0, testBuffer->GetInstanceCount());
        Console::log(std::format("Read back data from test buffer: {}, {}, {}, {}", resultData[0], resultData[1], resultData[2], resultData[3]), "Graphics");

        testBuffer->Flush();
        Console::log("Test buffer flushed", "Graphics");

        testBuffer->Invalidate();
        Console::log("Test buffer invalidated", "Graphics");

        testBuffer->Unmap();
        Console::log("Test buffer unmapped", "Graphics");
        testBuffer.reset();
    }

    void RunAllTests()
    {
        // TestBufferCreation();
        // TestBufferMapping();
        BasicBufferTest();
    }
}

#endif