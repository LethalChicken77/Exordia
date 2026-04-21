#pragma once
#include <allocators.h>
#include <span>
#include <vector>

namespace exo
{

/// @brief An alternative to std::vector that lives on the stack until it exceeds a certain number of elements.
/// @tparam T Type stored in the list.
/// @tparam SSize Stack size of the array. Default: 16
/// @tparam Alloc Allocator to use for heap allocations.
template<class T, size_t SSize = 16, class Alloc = std::allocator<T>>
class StackList
{
public:
    StackList() = default;

    // std::vector<T> vec;
    constexpr void Reserve(size_t size);
    constexpr void Resize(size_t size);
    // inline void test()
    // {
    //     vec.
    // }
private:
    std::array<T, SSize> m_stackData{};
    alignas(T) std::byte m_stackData[sizeof(T) * SSize];
    size_t m_size = 0;
    size_t m_capacity = SSize;
    std::span<T> m_heapData{};
    static const inline float s_growthFactor = 1.5f;
};
    
} // namespace exo