#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS 1
#endif

#include "floatCenterSlider.h"
#include <imgui_internal.h>

bool SliderFloatCenter(const char* label, float* v)
{
   return ImGui::SliderFloat(label, v, -100, 100, "%.3f", ImGuiSliderFlags_Logarithmic | ImGuiSliderFlags_NoInput);
}