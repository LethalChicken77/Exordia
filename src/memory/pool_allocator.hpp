#pragma once
#include <utility>
#include <memory>
#include <cassert>

/// @brief A simple pool allocator.
/// @tparam T Base type of the pool.
/// @note Using stale pointers after freeing causes undefined behavior. This allocator is not thread safe.
template<class T>
class PoolAllocator
{
public:
    // Everything lives in the header because template :D
    /// Create a new memory pool
    /// @param _capacity Number of elements of type T in the pool
    explicit PoolAllocator(size_t _capacity) : capacity(_capacity)
    {
        bufferSize = blockSize * capacity;
        memory = static_cast<std::byte*>(::operator new(bufferSize, std::align_val_t(alignof(T))));
        Reset();
    }
    ~PoolAllocator() { ::operator delete(memory, std::align_val_t(alignof(T))); }
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&& other) 
        : freeList(other.freeList), memory(other.memory), bufferSize(other.bufferSize), capacity(other.capacity)
    {
        other.memory = nullptr; // Ensure destructor of original object doesn't free the memory
        other.freeList = nullptr;
    }
    PoolAllocator& operator=(PoolAllocator&& other) = delete; // Only allow moving via std::move

    /// @brief Allocate a slot for an object of type T.
    /// @return Pointer to T memory. Returns nullptr if allocation failed.
    [[nodiscard]] T* Alloc() noexcept
    {
        if(freeList == nullptr)
        {
            return nullptr;
        }
        T* newT = reinterpret_cast<T*>(freeList);
        freeList = freeList->next;
        return newT;
    }
    /// @brief Allocate and construct an object of type T.
    /// @param ...args Arguments to be passed to the constructor of T.
    /// @return Pointer to newly constructed T. Returns nullptr if allocation failed.
    template<class... Args>
    [[nodiscard]] T* New(Args&&... args)
    {
        T* mem = Alloc();
        return mem ? new (mem) T(std::forward<Args>(args)...) : nullptr;
    }

    /// @brief Free the memory at a location and call the destructor.
    /// @param ptr Pointer to free.
    /// @note Invalidates pointer.
    void Free(T* ptr) noexcept
    {
        assert(memory <= ptr && ptr < memory + bufferSize && "Cannot free memory outside of buffer");
        assert((ptr - memory) % blockSize == 0 && "Cannot free misaligned memory within buffer");

        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = freeList;
        freeList = node;
    }

    /// @brief Free the memory at a location and call the destructor.
    /// @param ptr Pointer to delete.
    /// @note Invalidates pointer.
    void Delete(T* ptr)
    {
        assert(memory <= ptr && ptr < memory + bufferSize && "Cannot free memory outside of buffer");
        assert((ptr - memory) % blockSize == 0 && "Cannot free misaligned memory within buffer");
        
        ptr->~T();
        #ifdef DEBUG
        memset(ptr, 0xCD, blockSize); // Poison
        #endif

        FreeNode* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = freeList;
        freeList = node;
    }

    /// @brief Clear the contents of the pool without calling destructors.
    /// @note Invalidates all pointers.
    void Reset() noexcept
    {
        for(size_t i = 0; i < capacity; i++)
        {
            FreeNode* node = reinterpret_cast<FreeNode*>(memory + i * blockSize);
            node->next = (i + 1 < capacity) ? reinterpret_cast<FreeNode*>(memory + (i + 1) * blockSize) : nullptr;
        }
        freeList = reinterpret_cast<FreeNode*>(memory);
    }

    /// @brief Clear the contents of the pool, calling destructors.
    /// @note Invalidates all pointers.
    void ResetDelete()
    {
        // Flag free blocks
        std::vector<bool> isFree(capacity, false); // TODO: Investigate avoiding an allocation here
        FreeNode* current = freeList;
        while(current != nullptr)
        {
            size_t index = (reinterpret_cast<std::byte*>(current) - memory) / blockSize;
            isFree[index] = true;
            current = current->next;
        }

        // Iterate through and call destructor on any occupied blocks
        for(size_t i = 0; i < capacity; i++)
        {
            FreeNode* node = reinterpret_cast<FreeNode*>(memory + i * blockSize);
            if(isFree[i])
            {
                reinterpret_cast<T*>(node)->~T();
                #ifdef DEBUG
                memset(node, 0xCD, blockSize); // Poison
                #endif
            }
            node->next = (i + 1 < capacity) ? reinterpret_cast<FreeNode*>(memory + (i + 1) * blockSize) : nullptr;
        }
        freeList = reinterpret_cast<FreeNode*>(memory);
    }

    /// @brief Retrieve the value stored in a slot regardless of whether it is free.
    /// @param index Position in the pool
    /// @return Very unsafe pointer
    /// @note Abhorrently unsafe, mainly for debug purposes! Track yo pointers!
    T* Get(size_t index)
    {
       return reinterpret_cast<T*>(&memory[index * sizeof(T)]); 
    }

private:
    struct FreeNode
    {
        FreeNode* next = nullptr;
    };
    FreeNode* freeList;
    std::byte* memory;
    size_t bufferSize; // Size in bytes

    size_t capacity;
    // Wastes space if sizeof(T) < sizeof(FreeNode). Why are you using a pool for objects that are smaller than a pointer?
    static constexpr size_t blockSize = std::max(sizeof(T), sizeof(FreeNode));
};

template<class T>
using MemoryPool = PoolAllocator<T>;