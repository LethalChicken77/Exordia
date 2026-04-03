#pragma once
#include "modules.hpp"
#include "graphics/rendering/render_node.hpp"

namespace exo
{

class Runtime
{
public:
    void MainLoop();

private:
    graphics::RenderStage renderer3D;
};

} // namespace exo