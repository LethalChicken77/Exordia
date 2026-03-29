#include "arena_allocator.hpp"
#include <memory>
#include <cassert>

ArenaAllocator::ArenaAllocator(size_t size) 
    : capacity(size),
    offset(0)
{
    memory = static_cast<std::byte*>(::operator new(size, std::align_val_t(MAX_ALIGN)));
}

ArenaAllocator::~ArenaAllocator()
{
    ::operator delete(memory, std::align_val_t(MAX_ALIGN));
}

ArenaAllocator::ArenaAllocator(ArenaAllocator&& other)
    : capacity(other.capacity), offset(other.offset), memory(other.memory)
{
    other.memory = nullptr; // Ensure destructor of original object doesn't free the memory
}

/// @brief Allocate a chunk of memory.
/// @param size Size of requested memory in bytes.
/// @param alignment Alignment in bytes of whatever is being allocated. 
/// Defaults to maximum fundamental alignment. 
/// Assumed to be a power of 2.
/// @return Pointer to newly allocated memory. Returns nullptr if allocation failed.
void* ArenaAllocator::Alloc(size_t size, size_t alignment) noexcept
{
    assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2.");
    uintptr_t aligned = (reinterpret_cast<uintptr_t>(memory) + offset + alignment - 1) & ~(alignment - 1);

    size_t newOffset = (aligned - reinterpret_cast<uintptr_t>(memory)) + size;
    
    if(newOffset > capacity)
        return nullptr;
    
    offset = newOffset;
    return reinterpret_cast<void*>(aligned);
}

/// @brief Reset the allocator.
/// @note Does not clear existing data. 
void ArenaAllocator::Reset() noexcept
{
    offset = 0;
}

/// @brief Partially reset the allocator. 
/// @note Only use markers obtained from this allocator.
/// Invalidates all pointers and markers after marker. 
/// Usage with an arbitrary value will cause undefined behavior.
/// @param marker Marker to jump back to.
void ArenaAllocator::ResetTo(Marker marker) noexcept
{
    assert(marker.offset <= capacity && "Attempted to reset arena offset to invalid value.");
    offset = marker.offset;
}