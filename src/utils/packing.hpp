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

constexpr float UnpackHalfToFloat(uint16_t h)
{
    uint32_t sign = (uint32_t)(h & 0x8000u) << 16;
    uint32_t exp  = (h >> 10) & 0x1Fu;
    uint32_t mant = h & 0x03FFu;

    uint32_t fbits;

    if (exp == 0)
    {
        if (mant == 0)
        {
            // ±0
            fbits = sign;
        }
        else
        {
            // subnormal → normalize
            exp = 1;
            while ((mant & 0x0400u) == 0)
            {
                mant <<= 1;
                exp--;
            }

            mant &= 0x03FFu;
            exp = exp + (127 - 15);
            mant <<= 13;

            fbits = sign | (exp << 23) | mant;
        }
    }
    else if (exp == 31)
    {
        // Inf / NaN
        fbits = sign | 0x7F800000u | (mant << 13);
    }
    else
    {
        // normal
        exp = exp + (127 - 15);
        mant <<= 13;

        fbits = sign | (exp << 23) | mant;
    }

    return std::bit_cast<float>(fbits);
}

constexpr double UnpackHalfToDouble(uint16_t h)
{
    float f = UnpackHalfToFloat(h);
    return static_cast<double>(f);
}