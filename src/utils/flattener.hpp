#pragma once
#include <cstdint>
#include <glm/glm.hpp>

struct Flattener
{
    size_t width = 1;
    size_t height = 1;
    size_t depth = 1;

    constexpr Flattener(size_t _width, size_t _height, size_t _depth = 0)
        : width(_width), height(_height), depth(_depth) {}

    /// @brief Takes an integer position and returns an index.
    /// @param x 
    /// @param y 
    /// @return Flattened index
    constexpr size_t Flatten(size_t x, size_t y) const noexcept
    {
        return x + y * width;
    }
    /// @brief Takes an integer position and returns an index.
    /// @param x 
    /// @param y 
    /// @param z 
    /// @return Flattened index
    constexpr size_t Flatten(size_t x, size_t y, size_t z) const noexcept
    {
        return x + y * width + z * width * height;
    }

    /// @brief Takes an integer position and returns an index.
    /// @param pos
    /// @return Flattened index
    constexpr size_t Flatten(glm::i16vec2 pos) const noexcept
    {
        return Flatten(pos.x, pos.y);
    }

    /// @brief Takes an integer position and returns an index.
    /// @param pos
    /// @return Flattened index
    constexpr size_t Flatten(glm::i32vec2 pos) const noexcept
    {
        return Flatten(pos.x, pos.y);
    }

    /// @brief Takes an integer position and returns an index.
    /// @param pos
    /// @return Flattened index
    constexpr size_t Flatten(glm::i64vec2 pos) const noexcept
    {
        return Flatten(pos.x, pos.y);
    }

    /// @brief Takes an integer position and returns an index.
    /// @param pos
    /// @return Flattened index
    constexpr size_t Flatten(glm::i16vec3 pos) const noexcept
    {
        return Flatten(pos.x, pos.y, pos.z);
    }

    /// @brief Takes an integer position and returns an index.
    /// @param pos
    /// @return Flattened index
    constexpr size_t Flatten(glm::i32vec3 pos) const noexcept
    {
        return Flatten(pos.x, pos.y, pos.z);
    }

    /// @brief Takes an integer position and returns an index.
    /// @param pos
    /// @return Flattened index
    constexpr size_t Flatten(glm::i64vec3 pos) const noexcept
    {
        return Flatten(pos.x, pos.y, pos.z);
    }

    /// @brief Takes an index and returns a position.
    /// @tparam T Type of values to return. Valid types: glm::i16/32/64vec2, glm::i16/32/64vec2 
    /// @param index 
    /// @return Position 
    template <class T>
    T Unflatten(size_t index)
    {
        static_assert(std::is_same_v(T, glm::i16vec2()) || std::is_same_v(T, glm::i32vec2()) || std::is_same_v(T, glm::i64vec2())
            || std::is_same_v(T, glm::i16vec2()) || std::is_same_v(T, glm::i32vec2()) || std::is_same_v(T, glm::i64vec2()),
            "Unsupported type to unflatten to");
        
        if consteval (std::is_same_v(T, glm::i16vec2()) || std::is_same_v(T, glm::i32vec2()) || std::is_same_v(T, glm::i64vec2()))
        {
            return {index % width, index / width};
        }
        else
        {
            return {index % width, index / width % height, index / width / height};
        }
    }
};