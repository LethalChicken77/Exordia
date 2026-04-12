#pragma once
#include <glm/glm.hpp>

namespace graphics::Packing
{

/// @brief Pack a direction vector into an i8vec2. Assumes the vector is non-zero.
/// @param v Vector to pack.
/// @return Octahedral mapping of the vector v. Returns (0,0) if v is (0,0,0).
/// @note Unnormalized vectors work, though length information is lost. 
inline glm::i8vec2 PackOctahedral8(glm::vec3 v)
{
    float sum = glm::abs(v.x) + glm::abs(v.y) + glm::abs(v.z);
    glm::vec2 p = (sum > 0.f) ? glm::vec2(v.x, v.y) / sum : glm::vec2(0.f);

    if(v.z < 0.f)
    {
        p = (glm::vec2(1.f) - glm::abs(glm::vec2(p.y, p.x))) * glm::sign(p);
    }

    return glm::i8vec2(glm::round(p * 127.f)); // 128 goes unused
}

/// @brief Pack a direction vector into an i16vec2. Assumes the vector is non-zero.
/// @param v Vector to pack.
/// @return Octahedral mapping of the vector v. Returns (0,0) if v is (0,0,0).
/// @note Unnormalized vectors work, though length information is lost. 
inline glm::i16vec2 PackOctahedral16(glm::vec3 v)
{
    float sum = glm::abs(v.x) + glm::abs(v.y) + glm::abs(v.z);
    glm::vec2 p = (sum > 0.f) ? glm::vec2(v.x, v.y) / sum : glm::vec2(0.f);

    if(v.z < 0.f)
    {
        p = (glm::vec2(1.f) - glm::abs(glm::vec2(p.y, p.x))) * glm::sign(p);
    }

    return glm::i16vec2(glm::round(p * 32767.f));
}

};