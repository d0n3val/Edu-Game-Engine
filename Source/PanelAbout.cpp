#include "PanelAbout.h"
#include "Imgui/imgui.h"

// ---------------------------------------------------------
PanelAbout::PanelAbout() : Panel("About")
{
	active = false;
}

// ---------------------------------------------------------
PanelAbout::~PanelAbout()
{}

// ---------------------------------------------------------
void PanelAbout::Draw()
{
    ImGui::Begin("About EDU Engine", &active, 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoCollapse );

    ImGui::Text("Version %s", VERSION);
    ImGui::Separator();
    ImGui::Text("By Ricard Pillosu for teaching purposes.");
    ImGui::Text("EDU Engine is licensed under the Public Domain, see LICENSE for more information.");
    ImGui::End();
}
