#pragma once

#include <cstdint>
#include <bit> // for std::bit_cast (C++20)

constexpr uint16_t PackFloatToHalf(float value)
{
    uint32_t fbits = std::bit_cast<uint32_t>(value); // reinterpret float bits

    uint16_t sign = (fbits >> 16) & 0x8000u;
    int16_t exp = ((fbits >> 23) & 0xFF) - 127;
    uint32_t mant = fbits & 0x007FFFFFu;

    if (exp > 15)
        return sign | 0x7C00u; // clamp to +Inf
    else if (exp > -15)
    {
        exp += 15;
        mant >>= 13;
        return sign | (exp << 10) | mant;
    }
    else if (exp >= -24)
    {
        mant |= 0x00800000u;
        int shift = (-14 - exp);
        mant >>= (shift + 13);
        return sign | mant;
    }
    else
        return sign; // underflow
}

constexpr uint16_t PackFloatToHalf(double value)
{
    uint64_t dbits = std::bit_cast<uint64_t>(value);

    uint16_t sign = (dbits >> 48) & 0x8000u;
    int16_t exp = ((dbits >> 52) & 0x7FF) - 1023;
    uint64_t mant = dbits & 0xFFFFFFFFFFFFFull;

    if (exp > 15)
        return sign | 0x7C00u;
    else if (exp > -15)
    {
        exp += 15;
        mant >>= 42; // 52→10 bits
        return sign | static_cast<uint32_t>(mant);
    }
    else if (exp >= -24)
    {
        mant |= 0x10000000000000ull;
        int shift = (-14 - exp);
        mant >>= (shift + 42);
        return sign | static_cast<uint32_t>(mant);
    }
    else
        return sign; // underflow
}