#pragma once
#include <imgui.h>
// Extra utility functions for ImGui.

namespace ImGui
{

bool SliderDouble(const char* label, double* v, double v_min = 0.0f, double v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
bool DragDouble(const char* label, double* v, float v_speed = 1.0f, double v_min = 0.0f, double v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

bool SliderFloatN(const char* label, float* v, int v_components, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
bool DragFloatN(const char* label, float* v, int v_components, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
bool SliderDoubleN(const char* label, double* v, int v_components, double v_min = 0.0f, double v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
bool DragDoubleN(const char* label, double* v, int v_components, float v_speed = 1.0f, double v_min = 0.0f, double v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);

} // namespace ImGui