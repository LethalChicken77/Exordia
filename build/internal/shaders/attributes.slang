module attributes;

// Type attributes
[__AttributeUsage(_AttributeTargets.Var)]
public struct UNorm8Attribute {};
[__AttributeUsage(_AttributeTargets.Var)]
public struct SNorm8Attribute {};
[__AttributeUsage(_AttributeTargets.Var)]
public struct UNorm16Attribute {};
[__AttributeUsage(_AttributeTargets.Var)]
public struct SNorm16Attribute {};
[__AttributeUsage(_AttributeTargets.Var)]
public struct HalfAttribute {};

[__AttributeUsage(_AttributeTargets.Var)]
public struct ColorAttribute{};

// Range attributes
[__AttributeUsage(_AttributeTargets.Var)]
public struct RangeAttribute
{
    double min;
    double max;
}
[__AttributeUsage(_AttributeTargets.Var)]
public struct MaxAttribute
{
    double max;
}
[__AttributeUsage(_AttributeTargets.Var)]
public struct MinAttribute
{
    double min;
}

[__AttributeUsage(_AttributeTargets.Var)]
struct FRangeAttribute
{
    double min;
    double max;
}
[__AttributeUsage(_AttributeTargets.Var)]
struct IRangeAttribute
{
    int64_t min;
    int64_t max;
}
[__AttributeUsage(_AttributeTargets.Var)]
struct URangeAttribute
{
    uint64_t min;
    uint64_t max;
}

[__AttributeUsage(_AttributeTargets.Var)]
struct FMaxAttribute
{
    double max;
}
[__AttributeUsage(_AttributeTargets.Var)]
struct IMaxAttribute
{
    int64_t max;
}
[__AttributeUsage(_AttributeTargets.Var)]
struct UMaxAttribute
{
    uint64_t max;
}

[__AttributeUsage(_AttributeTargets.Var)]
struct FMinAttribute
{
    double min;
}
[__AttributeUsage(_AttributeTargets.Var)]
struct IMinAttribute
{
    int64_t min;
}
[__AttributeUsage(_AttributeTargets.Var)]
struct UMinAttribute
{
    uint64_t min;
}