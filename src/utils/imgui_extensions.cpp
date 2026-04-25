#include "imgui_extensions.hpp"

bool ImGui::SliderDouble(const char* label, double* v, double v_min, double v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderScalar(label, ImGuiDataType_Double, v, &v_min, &v_max, format, flags);
}

bool ImGui::DragDouble(const char* label, double* v, float v_speed, double v_min, double v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::DragScalar(label, ImGuiDataType_Double, v, v_speed, &v_min, &v_max, format, flags);
}



bool ImGui::SliderFloatN(const char* label, float* v, int v_components, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderScalarN(label, ImGuiDataType_Float, v, v_components, &v_min, &v_max, format, flags);
}

bool ImGui::DragFloatN(const char* label, float* v, int v_components, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::DragScalarN(label, ImGuiDataType_Float, v, v_components, v_speed, &v_min, &v_max, format, flags);
}



bool ImGui::SliderDoubleN(const char* label, double* v, int v_components, double v_min, double v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::SliderScalarN(label, ImGuiDataType_Double, v, v_components, &v_min, &v_max, format, flags);
}

bool ImGui::DragDoubleN(const char* label, double* v, int v_components, float v_speed, double v_min, double v_max, const char* format, ImGuiSliderFlags flags)
{
    return ImGui::DragScalarN(label, ImGuiDataType_Double, v, v_components, v_speed, &v_min, &v_max, format, flags);
}