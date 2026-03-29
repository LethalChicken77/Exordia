#pragma once
#include <cstddef>
#include <utility>

/// @brief An overwriting circular allocator. Useful for streaming data.
/// @note Overwrites existing data when the head wraps around. 
/// Ensure the capacity is large enough for your usage, or else you will read garbage data.
/// @warning NO SAFETY GUARANTEES
///    - Old allocations are silently overwritten, only use the most recent pointers!
class RingAllocator
{
    // TODO: Maybe rework to be more like a queue
public:
    explicit RingAllocator(size_t size);
    ~RingAllocator();
    RingAllocator(const RingAllocator&) = delete;
    RingAllocator& operator=(const RingAllocator&) = delete;
    RingAllocator(RingAllocator&&); // Allow std::move
    RingAllocator& operator=(RingAllocator&&) = delete;

    [[nodiscard]] void* Alloc(size_t size, size_t alignment = alignof(std::max_align_t)) noexcept;
    /// @tparam T Type to allocate.
    /// @brief Allocate a chunk of memory using a base type and a count.
    /// @param count Number of Ts to allocate. 
    /// @param alignment Alignment in bytes of whatever is being allocated. Defaults to alignment of T.
    /// @return Pointer to newly allocated memory. Returns nullptr if allocation failed.
    template<class T>
    [[nodiscard]] T* Alloc(size_t count = 1, size_t alignment = alignof(T)) noexcept
    {
        size_t size = sizeof(T) * count;
        return reinterpret_cast<T*>(Alloc(size, alignment));
    }

    /// @brief Allocate and construct an object of type T.
    /// @tparam T Type to construct.
    /// @param ...args Arguments to be passed to the constructor of T.
    /// @return Pointer to newly constructed T. Returns nullptr if allocation failed.
    /// @note Calling Reset() or ResetTo() will not call the destructors on allocated objects.
    /// Non-trivial destructors must be called manually.
    template<class T, class... Args>
    [[nodiscard]] T* New(Args&&... args)
    {
        T* mem = Alloc<T>(1);
        return mem ? new (mem) T(std::forward<Args>(args)...) : nullptr;
    }

    [[nodiscard]] size_t Capacity() const noexcept { return capacity; };
    [[nodiscard]] size_t Used() const noexcept { return offset; };
private:
    size_t capacity;
    size_t offset;

    std::byte* memory;

    static constexpr size_t MAX_ALIGN = 64;
};

using CircularAllocator = RingAllocator;
using MemoryRing = RingAllocator;