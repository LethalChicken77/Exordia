#include "ring_allocator.hpp"
#include <memory>
#include <cassert>

RingAllocator::RingAllocator(size_t size) 
    : capacity(size),
    offset(0)
{
    memory = static_cast<std::byte*>(::operator new(size, std::align_val_t(MAX_ALIGN)));
}

RingAllocator::~RingAllocator()
{
    ::operator delete(memory, std::align_val_t(MAX_ALIGN));
}

RingAllocator::RingAllocator(RingAllocator&& other)
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
void* RingAllocator::Alloc(size_t size, size_t alignment) noexcept
{
overflow:
    assert((alignment & (alignment - 1)) == 0 && "Alignment must be a power of 2.");
    assert(size <= capacity && "Cannot allocate space larger than the ring allocator."); // Prevent infinite loops
    uintptr_t aligned = (reinterpret_cast<uintptr_t>(memory) + offset + alignment - 1) & ~(alignment - 1);

    size_t newOffset = (aligned - reinterpret_cast<uintptr_t>(memory)) + size;
    
    if(newOffset > capacity)
    {
        offset = 0;
        goto overflow; // hehe I used a goto :D
    }
    
    offset = newOffset;
    return reinterpret_cast<void*>(aligned);
}