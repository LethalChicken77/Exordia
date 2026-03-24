#pragma once
#include <vector>
#include <cassert>

/// @brief Utility for flattening and retrieving 2D data
/// @tparam T
template<class T>
class Vector2D
{
public:
    Vector2D(size_t x, size_t y) : vec(x * y), width(x), height(y) 
    {
        assert((x > 0 && y > 0) && "Dimensions must be at least 1");
    }
    
    T& At(size_t x, size_t y) 
    {
        boundsCheck(x, y);
        return vec[index(x,y)]; 
    }

    size_t Width() const noexcept { return width; }
    size_t Height() const noexcept { return height; }
    size_t size() const noexcept { return vec.size(); }
    T* data() noexcept { return vec.data(); }
    const T* data() const noexcept { return vec.data(); }

    T& operator()(size_t x, size_t y)
    {
        boundsCheck(x, y);
        return vec[index(x, y)];
    }

    const T& operator()(size_t x, size_t y) const
    {
        boundsCheck(x, y);
        return vec[index(x, y)];
    }
    
    T& operator[](size_t i) noexcept { return vec[i]; }
    const T& operator[](size_t i) const noexcept { return vec[i]; }
private:
    std::vector<T> vec{};
    size_t width = 0;
    size_t height = 0;
    inline void boundsCheck(size_t x, size_t y) const
    {
        assert(x < width && "x is not within the width of the Vector2D");
        assert(y < height && "y is not within the height of the Vector2D");
    }

    inline size_t index(size_t x, size_t y) const noexcept
    {
        return x + y * width;
    }
};