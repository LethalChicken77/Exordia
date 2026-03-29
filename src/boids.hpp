#pragma once
#include <vector>
#include <memory>
#include "glm/glm.hpp"
#include "modules.hpp"
#include "graphics/api.hpp"


class BoidsSimulation
{
public:
    BoidsSimulation();
    void Update(float dt) noexcept;
    void DrawImGui();
    void Draw();

    void Randomize();
private:
    // Proxy for easy boid access, should be optimized out
    struct Boid
    {
        glm::vec3 &pos;
        glm::vec3 &vel;
    };
    static_assert(std::is_trivially_copyable_v<Boid>);

    struct SpatialEntry
    {
        uint32_t hash = 0;
        uint32_t index = 0;
    };

    struct Boids
    {
        std::vector<glm::vec3> pos{};
        std::vector<glm::vec3> vel{};
        std::vector<glm::mat4> transform{};
        std::vector<SpatialEntry> spatialLookup{};
        std::vector<SpatialEntry> spatialLookupTemp{};
        std::vector<uint32_t> startIndices{};
        uint32_t count;

        Boids(size_t _count)
        {
            pos = std::vector<glm::vec3>(_count);
            vel = std::vector<glm::vec3>(_count);
            transform = std::vector<glm::mat4>(_count);
            spatialLookup = std::vector<SpatialEntry>(_count);
            spatialLookupTemp = std::vector<SpatialEntry>(_count);
            startIndices = std::vector<uint32_t>(_count);
            count = _count;
        }

        [[nodiscard]] inline Boid operator[](uint32_t idx) noexcept
        {
            return { pos[idx], vel[idx] };
        }
    };

    struct Plane
    {
        glm::vec3 position;
        glm::vec3 normal;
    };

    struct Sphere
    {
        glm::vec3 position;
        float radius;
    };
    Boids boids{0};
    uint32_t numBoids = 100000;
    bool play = false;
    float propulsion = 0.01f;
    float cohesion = 0.2f; // Boids move towards each other
    float cohesionRadius = 0.375f;
    float cohesionAngle = 0.75f * glm::pi<float>();
    float cohesionCosAngle = 0;
    float cohesionExponent = 1;
    float avoidance = 3.f; // Boids avoid getting too close to each other
    float avoidanceRadius = 0.25f;
    float avoidanceExponent = 2;
    float avoidanceAngle = 0.75f * glm::pi<float>();
    float avoidanceCosAngle = 0;
    float alignment = 0.5f; // Boids turn to move in the same direction as each other
    float alignmentRadius = 0.375f;
    float alignmentExponent = 1;
    float alignmentAngle = 0.6f * glm::pi<float>();
    float alignmentCosAngle = 0;
    float nearRepelRadius = 0.05f;
    float nearRepelExponent = 0.5f;
    float nearRepelStrength = 2000.f;
    // float dt = 0.2f;
    float maxSpeed = 1;
    float cellSize;

    float xWall = 10;
    float yWall = 10;
    float zWall = 10;
    float wallRepulsion = 400.f;
    uint32_t nBalls = 50;
    float maxBallRadius = 4.f;
    std::vector<Plane> walls{6};
    std::vector<glm::mat4> wallTransforms{};
    std::vector<Sphere> balls{};

    core::Mesh boidMesh{};
    core::Mesh ballMesh{};
    core::Mesh wallMesh{};
    // std::unique_ptr<graphics::Material> boidMat{};

    void reset();
    void createWalls();
    void createBalls();

    uint32_t spatialHash(glm::vec3 pos, float cellSize);
    void radixSort(std::vector<SpatialEntry> &src, std::vector<SpatialEntry> &dst);
    void createSpatialLookup();

    void updateVel(glm::vec3& pos, glm::vec3& vel, glm::vec3& otherPos, glm::vec3& otherVel, glm::vec3& netForce);
    void updateAll(uint32_t i, glm::vec3& netForce);
    void updateLocal(uint32_t i, glm::vec3& netForce);
    void applyForceAll();
};