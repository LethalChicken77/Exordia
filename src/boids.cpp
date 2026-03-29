#include "boids.hpp"
#include "random.hpp"
#include "core/temp_data.hpp"
#include <omp.h>
#include <thread>

using namespace core;

BoidsSimulation::BoidsSimulation()
{
    // Returns the number of concurrent threads supported by the implementation. The value should be considered only a hint.
	unsigned int num_threads_available = std::thread::hardware_concurrency();
	std::cout << "Detected " << num_threads_available << " Availible Concurrent Threads" << std::endl;
	// Tell OpenMP how many threads we want to use, usually having a spare thread (or more) to ensure non-blocking/context switches
	unsigned int num_threads_using = std::max(num_threads_available - 1u, 1u);
	omp_set_num_threads(num_threads_using);
    #pragma omp parallel
    {
        #pragma omp single
        std::cout << "OpenMP using " << omp_get_num_threads() << " Threads\n";
    }

    MeshImportOptions meshImport{};
    meshImport.importScale = glm::vec3(0.05f);
    boidMesh = Mesh::loadObj("internal/models/boid.obj", "Boid mesh", meshImport);
    ballMesh = Mesh::loadObj("internal/models/ball.obj");
    wallMesh = Mesh::createGrid(16,16, {1.0f, 1.0f});
    
    graphicsModule.RegisterMesh(*boidMesh);
    graphicsModule.RegisterMesh(*ballMesh);
    graphicsModule.RegisterMesh(*wallMesh);

    reset();
}

void BoidsSimulation::reset()
{
    boids = Boids(numBoids);
#pragma omp parallel for
    for(uint32_t i = 0; i < boids.count; i++)
    {
        glm::vec3 &pos = boids.pos[i];
        pos.x = Random::getRandomFloat(-xWall, xWall);
        pos.y = Random::getRandomFloat(-yWall, yWall);
        pos.z = Random::getRandomFloat(-zWall, zWall);
        glm::vec3 &vel = boids.vel[i];
        vel = Random::InsideUnitSphere() * maxSpeed;
    }

    createWalls();
    createBalls();
}

void BoidsSimulation::createWalls()
{
    walls[0] = { glm::vec3(-xWall,0,0), glm::vec3(1,0,0) };
    walls[1] = { glm::vec3(xWall,0,0),  glm::vec3(-1,0,0) };
    walls[2] = { glm::vec3(0,-yWall,0), glm::vec3(0,1,0) };
    walls[3] = { glm::vec3(0,yWall,0),  glm::vec3(0,-1,0) };
    walls[4] = { glm::vec3(0,0,-zWall), glm::vec3(0,0,1) };
    walls[5] = { glm::vec3(0,0,zWall),  glm::vec3(0,0,-1) };

    wallTransforms.clear();
    // -X wall
    {
        glm::mat4 t(1.0f);
        t = glm::scale(glm::mat4(1), glm::vec3(2*yWall, 1.0f, 2*zWall)) * t;
        t = glm::translate(glm::mat4(1), glm::vec3(0, -xWall, 0)) * t;
        t = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0,0,1)) * t;
        wallTransforms.push_back(t);
    }

    // +X wall
    {
        glm::mat4 t(1.0f);
        t = glm::scale(glm::mat4(1), glm::vec3(2*yWall, 1.0f, 2*zWall)) * t;
        t = glm::translate(glm::mat4(1), glm::vec3(0, -xWall, 0)) * t;
        t = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0,0,1)) * t;
        wallTransforms.push_back(t);
    }

    // -Y wall
    {
        glm::mat4 t(1.0f);
        t = glm::scale(glm::mat4(1), glm::vec3(2*xWall, 1.0f, 2*zWall)) * t;
        t = glm::translate(glm::mat4(1), glm::vec3(0, -yWall, 0)) * t;
        // t = glm::rotate(t, glm::radians(90.0f), glm::vec3(0,0,1));
        wallTransforms.push_back(t);
    }

    // +Y wall
    {
        glm::mat4 t(1.0f);
        t = glm::scale(glm::mat4(1), glm::vec3(2*xWall, 1.0f, 2*zWall)) * t;
        t = glm::translate(glm::mat4(1), glm::vec3(0, -yWall, 0)) * t;
        t = glm::rotate(glm::mat4(1), glm::radians(180.0f), glm::vec3(1,0,0)) * t;
        wallTransforms.push_back(t);
    }

    // -Z wall
    {
        glm::mat4 t(1.0f);
        t = glm::scale(glm::mat4(1), glm::vec3(2*xWall, 1.0f, 2*yWall)) * t;
        t = glm::translate(glm::mat4(1), glm::vec3(0, -zWall, 0)) * t;
        t = glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(1,0,0)) * t;
        wallTransforms.push_back(t);
    }

    // +Z wall
    {
        glm::mat4 t(1.0f);
        t = glm::scale(glm::mat4(1), glm::vec3(2*xWall, 1.0f, 2*yWall)) * t;
        t = glm::translate(glm::mat4(1), glm::vec3(0, -zWall, 0)) * t;
        t = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1,0,0)) * t;
        wallTransforms.push_back(t);
    }
}

void BoidsSimulation::createBalls()
{
    balls.clear();
    for(uint32_t i = 0; i < nBalls; i++)
    {
        Sphere& s = balls.emplace_back();
        float x = Random::getRandomFloat(-1, 1);
        float y = Random::getRandomFloat(-1, 1);
        float z = Random::getRandomFloat(-1, 1);
        x = glm::sqrt(glm::abs(x)) * glm::sign(x);
        y = glm::sqrt(glm::abs(y)) * glm::sign(y);
        z = glm::sqrt(glm::abs(z)) * glm::sign(z);
        s.position = glm::vec3(
            x * xWall,
            y * yWall,
            z * zWall
        );
        s.radius = Random::getRandomFloat(0.5, 1);
        s.radius *= s.radius * s.radius * maxBallRadius;
    }
}

float spikeFunc(float t, float radius)
{
    return glm::max(1.f - t, 0.f);
    // return glm::max(1.f - glm::abs(t), 0.f);
}

void BoidsSimulation::updateVel(glm::vec3& pos, glm::vec3& vel, glm::vec3& otherPos, glm::vec3& otherVel, glm::vec3& netForce)
{
    glm::vec3 delta = otherPos - pos;
    float distance = glm::length(delta);
    if(distance < 0.000001f)
        return;
    glm::vec3 dir = delta / distance;
    float cosAngle = glm::dot(glm::normalize(vel), dir);
    if(distance < nearRepelRadius)
    {
        // vel += -dir * glm::pow(spikeFunc(distance, nearRepelRadius) * nearRepelStrength * dt, nearRepelExponent);
        netForce += -dir * spikeFunc(distance, nearRepelRadius) * nearRepelStrength;
        // netForce += -dir * nearRepelStrength;
    }
    if(distance < cohesionRadius && cosAngle > cohesionCosAngle)
    {
        // netForce += dir * spikeFunc(distance, cohesionRadius) * cohesion;
        netForce += dir * cohesion;
    }
    if(distance < avoidanceRadius && cosAngle > avoidanceCosAngle)
    {
        netForce += -dir * glm::pow(spikeFunc(distance, avoidanceRadius), avoidanceExponent) * avoidance;
    }
    if(distance < alignmentRadius && cosAngle > alignmentCosAngle)
    {
        netForce += otherVel * spikeFunc(distance, avoidanceRadius) * alignment;
    }
}

void BoidsSimulation::updateAll(uint32_t i, glm::vec3& netForce)
{
    for(uint32_t j = 0; j < boids.count; j++)
    {
        if(i == j) continue;
        updateVel(boids[i].pos, boids[i].vel, boids[j].pos, boids[j].vel, netForce);
    }
}

void BoidsSimulation::updateLocal(uint32_t i, glm::vec3& netForce)
{
    for(int z = -1; z < 2; z++)
    {
        for(int y = -1; y < 2; y++)
        {
            for(int x = -1; x < 2; x++)
            {
                glm::vec3 cellOffset = glm::vec3(x, y, z) * cellSize;
                uint32_t cellHash = spatialHash(boids[i].pos + cellOffset, cellSize);
                uint32_t index = boids.startIndices[cellHash];
                if(index == ~0u) continue; // Invalid start index
                while(index < boids.count && boids.spatialLookup[index].hash == cellHash)
                {
                    uint32_t j = boids.spatialLookup[index].index;
                    if(i != j)
                        updateVel(boids[i].pos, boids[i].vel, boids[j].pos, boids[j].vel, netForce);
                    index++;
                }
            }
        }
    }

    // for(uint32_t j = 0; j < boids.count; j++)
    // {
    // }
}

void BoidsSimulation::Update(float dt) noexcept
{
    if(!play) return;
    createSpatialLookup();
#pragma omp parallel for
    for(uint32_t i = 0; i < boids.count; i++)
    {
        glm::vec3& vel = boids[i].vel;
        glm::vec3 pos = boids[i].pos + vel * dt;

        glm::vec3 netForce = glm::vec3(0);
        for(Plane &plane : walls)
        {
            float penetration = glm::dot(pos + plane.position, plane.normal);
            if(penetration > 0)
            {
                netForce -= plane.normal * (penetration + 0.01f) * wallRepulsion;
            }
        }

        for(Sphere &ball : balls)
        {
            glm::vec3 delta = pos - ball.position;
            float dist = glm::length(delta);
            float penetration = ball.radius - dist;
            if(penetration > 0)
            {
                glm::vec3 dir = delta / dist;
                netForce += dir * (penetration + 0.01f) * wallRepulsion;
            }
        }

        // updateAll(i, netForce);
        updateLocal(i, netForce);
        
        netForce += glm::normalize(vel) * propulsion;
        vel += netForce * dt;
        float speed = glm::length(vel);
        if(speed > maxSpeed)
        {
            vel -= glm::normalize(vel) * (speed - maxSpeed);
        }
    }
#pragma omp parallel for
    for(uint32_t i = 0; i < boids.count; i++) // Update positions after for stability
    {
        glm::vec3& pos = boids[i].pos;
        glm::vec3& vel = boids[i].vel;

        pos += vel * dt;
    }
}

void BoidsSimulation::Draw()
{
#pragma omp parallel for
    for(uint32_t i = 0; i < boids.count; i++)
    {
        glm::mat4 &t = boids.transform[i];
        t = glm::mat4(1);
        glm::vec3 facing = glm::normalize(boids.vel[i]);
        glm::vec3 wup = glm::vec3(0,1,0);
        if (glm::abs(glm::dot(facing, wup)) > 0.999f)
            wup = glm::vec3(1,0,0);

        glm::vec3 right = glm::normalize(glm::cross(wup, facing));
        glm::vec3 up = glm::normalize(glm::cross(facing, right));
        t[0] = glm::vec4(right, 0);
        t[1] = glm::vec4(up, 0);
        t[2] = glm::vec4(facing, 0);
        t[3] = glm::vec4(boids.pos[i], 1);
    }

    graphicsModule.DrawMesh(boidMesh, *core::gameData->materials.Get(0), boids.transform);
    for(Sphere &s : balls)
    {
        glm::mat4 t = glm::mat4(s.radius);
        t[3] = glm::vec4(s.position.x, s.position.y, s.position.z, 1);
        graphicsModule.DrawMesh(ballMesh, *core::gameData->materials.Get(1), t);
    }

    graphicsModule.DrawMesh(wallMesh, *core::gameData->materials.Get(2), wallTransforms);
}

void BoidsSimulation::DrawImGui()
{
    ImGui::Begin("Boids");

    ImGui::Text("Simulation Settings");
    ImGui::InputInt("Num Boids", (int*)&numBoids);
    
    glm::vec3 arenaSize = { xWall / 2.f, yWall / 2.f, zWall / 2.f };
    ImGui::DragFloat3("Arena Size", (float*)&arenaSize);
    xWall = arenaSize.x * 2;
    yWall = arenaSize.y * 2;
    zWall = arenaSize.z * 2;
    createWalls();
    int t_nBalls = nBalls;
    ImGui::InputInt("N Balls", &t_nBalls);
    nBalls = glm::max(t_nBalls, 0);
    ImGui::DragFloat("Max Ball Radius", &maxBallRadius);
    maxBallRadius = glm::max(maxBallRadius, 0.f);

    // ImGui::SliderFloat("Time Step", &dt, 0.001, 1);
    ImGui::Checkbox("Play", &play);
    if(ImGui::SmallButton("Reset"))
    {
        reset();
    }
    if(ImGui::CollapsingHeader("Boid Settings"))
    {
        ImGui::DragFloat("Max Speed", &maxSpeed);
        maxSpeed = glm::max(maxSpeed, 0.f);
        ImGui::DragFloat("Propulsion", &propulsion);
        propulsion = glm::max(propulsion, 0.f);
        ImGui::Spacing();
        ImGui::DragFloat("Cohesion Radius", &cohesionRadius, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("Cohesion Strength", &cohesion, 0.01f);
        ImGui::SliderAngle("Cohesion Angle", &cohesionAngle, 0, 180);
        cohesionCosAngle = glm::cos(cohesionAngle);
        ImGui::Spacing();
        ImGui::DragFloat("Alignment Radius", &alignmentRadius, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("Alignment Strength", &alignment, 0.01f);
        ImGui::SliderAngle("Alignment Angle", &alignmentAngle, 0, 180);
        alignmentCosAngle = glm::cos(alignmentAngle);
        ImGui::Spacing();
        ImGui::DragFloat("Avoidance Radius", &avoidanceRadius, 0.01f, 0.f, 1.f);
        ImGui::DragFloat("Avoidance Strength", &avoidance, 0.01f);
        ImGui::SliderAngle("Avoidance Angle", &avoidanceAngle, 0, 180);
        avoidanceCosAngle = glm::cos(avoidanceAngle);
        ImGui::Spacing();
        ImGui::DragFloat("Near Force Radius", &nearRepelRadius, 0.001f, 0.f, 0.5f);
        ImGui::DragFloat("Near Force Strength", &nearRepelStrength, 1.f, 0.f);
        ImGui::DragFloat("Collision Strength", &wallRepulsion, 1.f, 0.f);
    }

    ImGui::End();
}

uint32_t BoidsSimulation::spatialHash(glm::vec3 pos, float cellSize)
{
    glm::ivec3 iPos = glm::floor(pos / cellSize);
    // Large primes for each axis
    const uint32_t p1 = 73856093u;
    const uint32_t p2 = 19349663u;
    const uint32_t p3 = 83492791u;

    // Mix coordinates, ensure positive
    uint32_t x = iPos.x;
    uint32_t y = iPos.y;
    uint32_t z = iPos.z;

    // XOR the products
    return ((x * p1) ^ (y * p2) ^ (z * p3)) % boids.count;
}

void BoidsSimulation::radixSort(std::vector<SpatialEntry> &src, std::vector<SpatialEntry> &dst)
{
    const uint32_t n = (uint32_t)src.size();

    constexpr uint32_t RADIX = 256;
    constexpr uint32_t MASK  = RADIX - 1;

    uint32_t count[RADIX];

    // Flip initial roles
    SpatialEntry* in  = dst.data();
    SpatialEntry* out = src.data();

    // First pass reads from src, so copy pointers accordingly
    // (we just swap once logically)
    in  = src.data();
    out = dst.data();

    for(uint32_t pass = 0; pass < 4; pass++)
    {
        uint32_t shift = pass * 8;

        memset(count, 0, sizeof(count));

        // Histogram
        for(uint32_t i = 0; i < n; i++)
        {
            uint32_t digit = (in[i].hash >> shift) & MASK;
            count[digit]++;
        }

        // Prefix sum
        uint32_t sum = 0;
        for(uint32_t i = 0; i < RADIX; i++)
        {
            uint32_t c = count[i];
            count[i] = sum;
            sum += c;
        }

        // Scatter
        for(uint32_t i = 0; i < n; i++)
        {
            uint32_t digit = (in[i].hash >> shift) & MASK;
            out[count[digit]++] = in[i];
        }

        // Swap buffers
        SpatialEntry* tmp = in;
        in  = out;
        out = tmp;
    }
}

void BoidsSimulation::createSpatialLookup()
{
    cellSize = glm::max(alignmentRadius, cohesionRadius, avoidanceRadius);
#pragma omp parallel for
    for(uint32_t i = 0; i < boids.count; i++)
    {
        glm::vec3& pos = boids[i].pos;
        
        boids.spatialLookup[i] = {spatialHash(pos, cellSize), i};
    }

    // Sort based on hashes
    // std::sort(boids.spatialLookup.begin(), boids.spatialLookup.end(), [](const auto &a, const auto &b) {
    //     return a.hash < b.hash;
    // });
    radixSort(boids.spatialLookup, boids.spatialLookupTemp);

    std::fill(boids.startIndices.begin(), boids.startIndices.end(), ~0u);
    for(uint32_t i = 0; i < boids.count; i++)
    {
        uint32_t hash = boids.spatialLookup[i].hash;

        if(i == 0 || boids.spatialLookup[i - 1].hash != hash)
        {
            boids.startIndices[hash] = i;
        }
    }
}