// Inspired by Hazel's reference implementation by "The Cherno"
#pragma once
#include <cstdint>
#include <atomic>
#include <type_traits>

/// @brief Include as private.
class RefCounted
{
public:
    RefCounted() = default;
    virtual ~RefCounted() = default;

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

    template<class T>
    friend class Ref;

    static RefCounted* as_refcounted(void* ptr) noexcept
    {
        return static_cast<RefCounted*>(ptr);   // this line is now allowed inside RefCounted
    }

    static const RefCounted* as_refcounted(const void* ptr) noexcept
    {
        return static_cast<const RefCounted*>(ptr);
    }
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

    Ref(const Ref<T>& other) noexcept : m_data(other.m_data)
    {
        acquire();
    }

    Ref(Ref<T>&& other) noexcept : m_data(other.m_data)
    {
        other.m_data = nullptr;
    }
    
    Ref<T>& operator=(const Ref<T>& other) noexcept
    {
        if (this != &other)
        {
            release();
            m_data = other.m_data;
            acquire();
        }
        return *this;
    }

    Ref<T>& operator=(Ref<T>&& other) noexcept
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
    T* operator->() const noexcept { return m_data; }
    bool operator==(const Ref<T>& other) const { return m_data == other.m_data; }
    bool operator!=(const Ref<T>& other) const { return m_data != other.m_data; }
    
private: 
    T* m_data = nullptr;

    void acquire() noexcept
    {
        if(m_data)
            RefCounted::as_refcounted(m_data)->AddRef();
    }

    void release() noexcept
    {
        if(m_data)
        {
            if(RefCounted::as_refcounted(m_data)->RemoveRef() == 0)
                delete m_data;
            m_data = nullptr;
        }
    }
};