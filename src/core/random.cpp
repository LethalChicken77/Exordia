#include "random.hpp"

namespace core
{

    std::mt19937 rng{};
    std::random_device Random::rd{};
    
} // namespace core