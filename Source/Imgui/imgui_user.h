#pragma once

#include "imgui.h"
#include "imgui_internal.h"

namespace ImGui
{

bool SliderAngle3(const char* label, float* v_rad, float v_degrees_min = 0.f, float v_degrees_max = 360.f);

int Curve(const char *label, const ImVec2& size, int maxpoints, ImVec2 *points);
float CurveValue(float p, int maxpoints, const ImVec2 *points);

} // namespace ImGui