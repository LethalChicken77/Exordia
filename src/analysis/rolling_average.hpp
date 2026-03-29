#pragma once
#include <type_traits>
#include <vector>

namespace analysis
{

template<typename T>
class RollingAverage
{
static_assert(std::is_arithmetic_v<T>);
public:
    RollingAverage(uint32_t numVals) : vals(numVals) {};

    void PushValue(T val)
    {
        T oldVal = vals[head];
        vals[head] = val;
        if(written == vals.size())
            sum -= oldVal;
        else if(written < vals.size())
            written++;
        head = (head + 1) % vals.size();
        sum += val;
    }

    float GetAverage() const
    {
        return sum / written;
    }
private:
    std::vector<T> vals{};
    uint32_t head = 0;
    uint32_t written = 0;
    float sum = 0;
};

template<typename T>
class ExponentialRollingAverage
{
static_assert(std::is_arithmetic<T>());
public:
    ExponentialRollingAverage(uint32_t numVals, float decay = 0.5f) : vals(numVals), alpha(decay) {};

    void PushValue(T val)
    {
        vals[head] = val;
        if(written < vals.size())
            written++;
        head = (head + 1) % vals.size();
    }

    float GetAverage() const
    {
        if(vals.size() == 0) return 0;
        float prev = vals[head];
        for(uint32_t i = 1; i < written; i++)
        {
            uint32_t headI = (head + i) % written;
            float fVal = static_cast<float>(vals[headI]);
            prev = alpha * vals[headI] + (1 - alpha) * prev;
        }
        return prev;
    }
private:
    std::vector<T> vals{};
    float alpha = 1;
    uint32_t head = 0;
    uint32_t written = 0;
};

} // namespace analysis