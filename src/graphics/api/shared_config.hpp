#pragma once

namespace graphics
{

enum class CompareOp : unsigned char
{
    Never = 0,
    Less = 1,
    LessEqual = 2,
    Equal = 3,
    GreaterEqual = 4,
    Greater = 5,
    NotEqual = 6,
    Always = 7
};

} // namespace graphics