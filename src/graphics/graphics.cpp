#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"
#include "backend/buffer.hpp"

namespace graphics
{
    std::unique_ptr<Buffer> testBuffer;
    Graphics::Graphics(const std::string& appName, const std::string& engName)
    {
        Console::log("Initializing graphics module", "Graphics");
        graphicsData = std::make_unique<GraphicsData>();
        graphicsData->window.Init(800, 600, engName + " - " + appName);
        graphicsData->backend.Init(appName, engName, graphicsData->GetGLFWWindow());

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
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
    }
} // namespace graphics