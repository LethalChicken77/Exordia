#include "graphics.hpp"
#include "graphics_data.hpp"
#include "utils/console.hpp"
#include "backend/buffer.hpp"

#include "graphics_tests.hpp"

namespace graphics
{
    std::unique_ptr<Buffer> testBuffer;
    Graphics::Graphics(const std::string& appName, const std::string& engName)
    {
        Console::log("Initializing graphics module", "Graphics");
        graphicsData = std::make_unique<GraphicsData>();
        graphicsData->window.Init(800, 600, engName + " - " + appName);
        graphicsData->backend.Init(appName, engName, graphicsData->GetGLFWWindow());
        
        #ifdef GRAPHICS_TESTS
        tests::RunAllTests();
        #endif
    }

    Graphics::~Graphics()
    {
        Console::log("Shutting down graphics module", "Graphics");
    }
} // namespace graphics