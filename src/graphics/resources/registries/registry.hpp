#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "registry.hpp"
#include "graphics/api/handles.hpp"

namespace graphics
{

/// @brief 
/// @tparam T Type being stored
/// @tparam H Handle type to use
template<class T, class H>
class GraphicsRegistry
{
public:
    // GraphicsRegistry(uint32_t maxEntries);

    // Add specialization functions to subclasses
    // Register()
    // Update()

    /// @brief Deregister a handle
    /// @param meshData 
    /// @return True if successful, false otherwise
    bool Deregister(H handle)
    {
        if(!IsValid(handle))
        {
            return false;
        }

        Entry &entry = entries[handle.index];
        entry.value.reset(); // TODO: Defer deletion to avoid errors during a frame
        entry.generation = ~0u;
        entry.inUse = false;
        
        return true;
    }

    /// @brief Get a pointer to a stored resource
    /// @param handle The handle of the resource to retrieve
    /// @return Pointer to the resource. nullptr if invalid.
    T* Get(H handle) const 
    {
        if(!IsValid(handle))
        {
            return nullptr;
        }
        return entries[handle.index].value.get();
    };

    /// @brief Check the validity of a handle
    /// @param handle
    /// @return True if valid, false otherwise
    bool IsValid(H handle) const
    {
        return handle.IsValid() && // Check if index is valid
            handle.index < entries.size() && // Check if index is in bounds
            entries[handle.index].inUse &&
            entries[handle.index].value != nullptr &&
            handle.generation == entries[handle.index].generation; // Check for UAF
    }

    /// @brief Delete all entries stored in the registry
    void Reset()
    {
        entries.clear();
        freeList.clear();
    }

protected:
    struct Entry
    {
        std::unique_ptr<T> value{}; // TODO: Look into removing pointer or allocating from a pool
        uint32_t generation = ~0u;
        bool inUse = false;
    };
    
    std::vector<Entry> entries{};
    std::vector<uint32_t> freeList{};

    uint32_t nextGeneration = 1;
};

} // namespace graphics