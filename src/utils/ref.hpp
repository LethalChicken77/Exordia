// Inspired by Hazel's reference implementation by "The Cherno"
#pragma once
#include <cstdint>
#include <atomic>
#include <memory>
#include <type_traits>

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WRef = std::weak_ptr<T>;

/// @brief Include as private.
class IRefCounted
{
public:
    IRefCounted() = default;
    virtual ~IRefCounted() = default;

    void AddRef() const
    {
        count.fetch_add(1, std::memory_order_relaxed);
    }

    uint32_t RemoveRef() const
    {
        return count.fetch_sub(1, std::memory_order_acq_rel);
    }

private:
    mutable std::atomic_uint32_t count = 0;

    template<std::derived_from<IRefCounted> T>
    friend class IRef;
};

// Intrusive reference counting reference
template<std::derived_from<IRefCounted> T>
class IRef
{
public:
    /// @brief Null constructor.
    IRef() = default;
    /// @brief Construct a new reference in-place.
    IRef(T&& rval) : m_data(new T(std::move(rval)))
    {
        acquire();
    }

    IRef(const IRef<T>& other) noexcept : m_data(other.m_data)
    {
        acquire();
    }

    IRef(IRef<T>&& other) noexcept : m_data(other.m_data)
    {
        other.m_data = nullptr;
    }
    
    IRef<T>& operator=(const IRef<T>& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_data = other.m_data;
            acquire();
        }
        return *this;
    }

    IRef<T>& operator=(IRef<T>&& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_data = other.m_data;
            other.m_data = nullptr;
        }
        return *this;
    }

    ~IRef() noexcept
    {
        release();
    }

    /// @brief Get a copy of the raw pointer.
    /// @return Pointer to underlying data.
    [[nodiscard]] T* Get() const noexcept { return m_data; }
    [[nodiscard]] bool IsNull() const noexcept { return m_data == nullptr; }

    explicit operator bool() const noexcept { return m_data != nullptr; }
    T& operator*() const noexcept { return *m_data; }
    T* operator->() const noexcept { return m_data; }
    bool operator==(const IRef<T>& other) const { return m_data == other.m_data; }
    bool operator!=(const IRef<T>& other) const { return m_data != other.m_data; }
    
private: 
    T* m_data = nullptr;

    void acquire() noexcept
    {
        if(m_data)
            static_cast<IRefCounted*>(m_data)->AddRef();
    }

    void release() noexcept
    {
        if(m_data)
        {
            if(static_cast<IRefCounted*>(m_data)->RemoveRef() == 1) // Compare to 1 as RemoveRef returns previous value
                delete m_data;
            m_data = nullptr;
        }
    }
};