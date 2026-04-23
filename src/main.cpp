#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <memory>

#include "close_handler.hpp"

#include "modules.hpp"
#include "utils/console.hpp"
#include "engine.hpp"
#include "utils/debug.hpp"
#include "core/input.hpp"


using namespace core;
using namespace graphics;


int main() 
{
    HandleSignals();
    // graphicsModule.init(APPLICATION_NAME, ENGINE_NAME);
    
    if(!graphicsModule.IsOpen())
    {
        Console::error("Failed to initialize graphics");
        return -1;
    }

    Engine engine{};

    engine.run();
    
    // graphicsModule.WaitForDevice();
    // graphicsModule.cleanup();

    return 0;
}