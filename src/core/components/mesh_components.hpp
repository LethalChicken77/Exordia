#include "core/mesh.hpp"
#include "core/assets/material.hpp"

namespace core
{
struct MeshRenderer
{
    MeshData mesh;
    id_t materialID{};
    // graphics::Material* material;
    // Cast shadows

};

struct MeshCollider
{
    MeshData mesh;
    // Physics material
    // Convex hull generation
};

} // namespace core