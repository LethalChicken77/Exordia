// Not functional yet, TODO: Slang reflection
// Include using #include rather than import for preprocessor defines
// #define RANGE(min, max) [Range(#min, #max)]

[__AttributeUsage(_AttributeTargets.Var)]
struct UNorm8Attribute {};
[__AttributeUsage(_AttributeTargets.Var)]
struct SNorm8Attribute {};
[__AttributeUsage(_AttributeTargets.Var)]
struct UNorm16Attribute {};
[__AttributeUsage(_AttributeTargets.Var)]
struct SNorm16Attribute {};

[__AttributeUsage(_AttributeTargets.Var)]
struct ColorAttribute{};

[__AttributeUsage(_AttributeTargets.Var)]
struct RangeAttribute // Parse in reflection
{
    string min;
    string max;
}

// [__AttributeUsage(_AttributeTargets.Var)]
// struct FRangeAttribute
// {
//     double min;
//     double max;
// }
// [__AttributeUsage(_AttributeTargets.Var)]
// struct IRangeAttribute
// {
//     int64_t min;
//     int64_t max;
// }
// [__AttributeUsage(_AttributeTargets.Var)]
// struct URangeAttribute
// {
//     uint64_t min;
//     uint64_t max;
// }
