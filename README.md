# Exordia
Exordium (noun), plural Exordia:
    A beginning or introduction especially to a discourse or *composition*
    -Merriam-Webster

Exordia is a modern graphics engine written in C++ using Vulkan. It is meant as a starting point for many projects, which is where the name comes from. I intend to expand the engine's capabilities through experiments ranging from graphics tech demos to physics simulation to small games. The end goal is a flexible, open source game engine with high fidelity graphics, modern C++ standards, and a customizable design.

## Features
3D rendering using Vulkan

Slang shaders

Compute shaders

Asset management system (Work in progress)

## Planned Features
Entity Component System hybrid architecture
- Object oriented for things with few instances, like players or game managers
- Data oriented for everything else, from enemies to projectiles
Particle Systems
2D Renderer
2D Lighting
2D Physics Engine
3D Physics Engine
Global illumination
Ray tracing
In-editor asset management
Multi-threaded design
3rd-party packages

## Build instructions
Build system: Cmake + Ninja + Clang
External dependencies can be found in CMakeLists.txt, ensure you have a recent version of Vulkan installed. I intend to improve the build system in the future, but in the meantime if you wish to use this as the starting point for your own projects you will have to figure out how to build it on your own. 
