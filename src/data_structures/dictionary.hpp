#pragma once
#include <optional>
#include <unordered_map>

namespace exo
{

/// @brief An improved version of std::unordered_map.
/// @tparam Key 
/// @tparam Value 
/// @note This is currently a wrapper around std::unordered_map. I plan to write my own implementation in the future.
template<class Key, class Value, class Hash = std::hash<Key>>
class Dictionary
{
public:
    std::optional<Value> Get(Key key) const noexcept;
private:
    std::unordered_map<Key, Value, Hash> map; // TODO: Custom implementation
};

} // namespace exo