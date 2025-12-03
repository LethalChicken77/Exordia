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
            1024,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );
        Console::log("Test buffer created", "Graphics");
        testBuffer->Map();
        Console::log("Test buffer mapped", "Graphics");
        testBuffer->Unmap();
        Console::log("Test buffer unmapped", "Graphics");
        testBuffer->Map();
        Console::log("Test buffer mapped again", "Graphics");
        testBuffer->Map();
        Console::log("Test buffer mapped again (Should error)", "Graphics");
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
    }
} // namespace graphics