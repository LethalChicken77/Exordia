// Inspired by Hazel's reference implementation by "The Cherno"
#pragma once
#include <cstdint>
#include <atomic>
#include <type_traits>

/// @brief Include as private.
class RefCounted
{
public:
    void AddRef() const
    {
        ++count;
    }

    uint32_t RemoveRef() const
    {
        return --count;
    }

private:
    mutable std::atomic_uint32_t count = 0;
};

template<class T>
class Ref
{
static_assert(std::is_base_of_v<RefCounted, T>, "Cannot create reference to class that does not derive from RefCounted.");
public:
    /// @brief Null constructor.
    Ref() = default;
    /// @brief Construct a new reference in-place.
    Ref(T&& rval) : m_data(new T(std::move(rval)))
    {
        acquire();
    }

    Ref(const Ref& other) noexcept : m_data(other.m_data)
    {
        acquire();
    }

    Ref(Ref&& other) noexcept : m_data(other.m_data)
    {
        other.m_data = nullptr;
    }
    
    Ref& operator=(const Ref& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_data = other.m_data;
            acquire();
        }
        return *this;
    }

    Ref& operator=(Ref&& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_data = other.m_data;
            other.m_data = nullptr;
        }
        return *this;
    }

    ~Ref() noexcept
    {
        release();
    }

    /// @brief Get a copy of the raw pointer.
    /// @return Pointer to underlying data.
    [[nodiscard]] T* Get() const noexcept { return m_data; }
    [[nodiscard]] bool IsNull() const noexcept { return m_data == nullptr; }

    explicit operator bool() const noexcept { return m_data != nullptr; }
    T& operator*() const noexcept { return *m_data; }
    T* operator->() { return *m_data; }
private: 
    T* m_data = nullptr;

    void acquire() noexcept
    {
        if(m_data)
            static_cast<RefCounted*>(m_data)->AddRef();
    }

    void release() noexcept
    {
        if(m_data)
        {
            if(static_cast<RefCounted*>(m_data)->RemoveRef() == 0)
                delete m_data;
            m_data = nullptr;
        }
    }
};