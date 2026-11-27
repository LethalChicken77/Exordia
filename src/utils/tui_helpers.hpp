#pragma once
#include "console.hpp"
#include "TuiScript.h"

namespace TuiHelpers
{

template<typename Enum>
inline bool ReadEnum(tui::table table, const char* key, Enum& value, Enum min, Enum max)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    if(!ref->isNumber())
        return false;

    double number = ref->numberValue();

    if(!std::isfinite(number) || std::trunc(number) != number ||
        number < static_cast<double>(min) || number > static_cast<double>(max))
        return false;

    value = static_cast<Enum>(static_cast<std::underlying_type_t<Enum>>(number));
    return true;
}

inline bool ReadBool(tui::table table, const char* key, bool& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    if(ref->type() != Tui_ref_type_BOOL)
        return false;

    value = ref->boolValue();
    return true;
}

inline bool ReadInt(tui::table table, const char* key, int64_t& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    if(!ref->isNumber())
        return false;

    double number = ref->numberValue();
    if(!std::isfinite(number))
        return false;

    int64_t converted = static_cast<int64_t>(number);
    if(!std::isfinite(converted))
        return false;

    value = converted;
    return true;
}

inline bool ReadFloat(tui::table table, const char* key, float& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    if(!ref->isNumber())
        return false;

    double number = ref->numberValue();
    if(!std::isfinite(number))
        return false;

    float converted = static_cast<float>(number);
    if(!std::isfinite(converted))
        return false;

    value = converted;
    return true;
}
inline bool ReadVec2(tui::table table, const char* key, glm::vec2& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;
    
    glm::dvec2 converted = ref->vec2Value();
    if(!std::isfinite(converted.x) || !std::isfinite(converted.y))
        return false;

    value = converted;
    return true;
}

inline bool ReadVec3(tui::table table, const char* key, glm::vec3& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    glm::dvec3 converted = ref->vec3Value();
    if(!std::isfinite(converted.x) || !std::isfinite(converted.y) || !std::isfinite(converted.z))
        return false;

    value = converted;
    return true;
}

inline bool ReadVec4(tui::table table, const char* key, glm::vec4& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    glm::dvec4 converted = ref->vec4Value();
    if(!std::isfinite(converted.x) || !std::isfinite(converted.y) || !std::isfinite(converted.z) || !std::isfinite(converted.w))
        return false;

    value = converted;
    return true;
}

inline bool ReadIVec2(tui::table table, const char* key, glm::ivec2& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    glm::dvec2 converted = ref->vec2Value();
    for(int i = 0; i < 2; i++)
    {
        if(!std::isfinite(converted[i]) || converted[i] < std::numeric_limits<int>::min() || converted[i] > std::numeric_limits<int>::max())
            return false;
    }

    value = converted;
    return true;
}

inline bool ReadIVec3(tui::table table, const char* key, glm::ivec3& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    glm::dvec3 converted = ref->vec3Value();
    for(int i = 0; i < 3; i++)
    {
        if(!std::isfinite(converted[i]) || converted[i] < std::numeric_limits<int>::min() || converted[i] > std::numeric_limits<int>::max())
            return false;
    }

    value = converted;
    return true;
}

inline bool ReadIVec4(tui::table table, const char* key, glm::ivec4& value)
{
    tui::ref ref = table->get(key);
    if(!ref)
        return false;

    glm::dvec4 converted = ref->vec4Value();
    for(int i = 0; i < 4; i++)
    {
        if(!std::isfinite(converted[i]) || converted[i] < std::numeric_limits<int>::min() || converted[i] > std::numeric_limits<int>::max())
            return false;
    }

    value = converted;
    return true;
}

}