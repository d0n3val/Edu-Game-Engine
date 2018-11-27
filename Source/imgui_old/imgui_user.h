#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#define IMGUI_GREY ImVec4(0.6f,0.6f,0.6f,1.f)
#define IMGUI_BLUE ImVec4(0.2f,0.2f,1.f,1.f)
#define IMGUI_GREEN ImVec4(0.f,1.f,0.f,1.f)
#define IMGUI_YELLOW ImVec4(1.f,1.f,0.f,1.f)
#define IMGUI_RED ImVec4(1.f,0.f,0.f,1.f)
#define IMGUI_WHITE ImVec4(1.f,1.f,1.f,1.f)

#define IMGUI_PRINT(field, format, ...) \
	ImGui::Text(field); \
	ImGui::SameLine(); \
	ImGui::TextColored(IMGUI_YELLOW, format, __VA_ARGS__)

namespace ImGui
{

bool SliderAngle3(const char* label, float* v_rad, float v_degrees_min = 0.f, float v_degrees_max = 360.f);

int Curve(const char *label, const ImVec2& size, int maxpoints, ImVec2 *points);
float CurveValue(float p, int maxpoints, const ImVec2 *points);

} // namespace ImGui