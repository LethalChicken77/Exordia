#pragma once
#include <cstddef>
#include <utility>

/// @brief A simple, allocator with fast allocation, but can only be deallocated all at once.
/// @note Using stale pointers after calling Reset() causes undefined behavior. This allocator is not thread safe.
class ArenaAllocator
{
public:
    // Wrapper for a marker to ensure it can only be created by the allocator.
    class Marker
    {
    public:
        Marker(const Marker&) = default;
        Marker& operator=(const Marker&) = default;
    private:
        friend class ArenaAllocator;
        Marker(size_t _offset) : offset(_offset) {}
        size_t offset;
    };

    // RAII utility, automatically resets allocations when leaving scope.
    struct [[nodiscard]] Scope
    {
        ArenaAllocator& arena;
        Marker marker;

        Scope(ArenaAllocator &_arena) noexcept
            : arena(_arena),
            marker(_arena.GetMarker()) {}

        ~Scope() noexcept { arena.ResetTo(marker); }

        Scope(const Scope&) = delete;
        Scope& operator=(const Scope&) = delete;
    };
public:
    explicit ArenaAllocator(size_t size);
    ~ArenaAllocator();
    ArenaAllocator(const ArenaAllocator&) = delete;
    ArenaAllocator& operator=(const ArenaAllocator&) = delete;
    ArenaAllocator(ArenaAllocator&&) { memory = nullptr; }
    ArenaAllocator& operator=(ArenaAllocator&&) = default;

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

    /// @brief Get a marker storing the current offset into the memory region.
    [[nodiscard]] Marker GetMarker() const noexcept { return Marker(offset); }
    void Reset() noexcept;
    void ResetTo(Marker marker) noexcept;

    [[nodiscard]] size_t Capacity() const noexcept { return capacity; };
    [[nodiscard]] size_t Used() const noexcept { return offset; };
    [[nodiscard]] size_t Remaining() const noexcept { return capacity - offset; };
private:
    size_t capacity;
    size_t offset;

    std::byte* memory;

    static constexpr size_t MAX_ALIGN = 64;
};

using MemoryArena = ArenaAllocator;