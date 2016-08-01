#include "PanelProperties.h"
#include "Imgui/imgui.h"
#include "GameObject.h"

// ---------------------------------------------------------
PanelProperties::PanelProperties() : Panel("Properties", SDL_SCANCODE_F3)
{}

// ---------------------------------------------------------
PanelProperties::~PanelProperties()
{}

// ---------------------------------------------------------
void PanelProperties::Draw()
{
    ImGui::Begin("Properties", &active, ImGuiWindowFlags_NoFocusOnAppearing);

	if (selected != nullptr)
	{
		// Active check box
		bool active = selected->IsActive();
		ImGui::Checkbox(" ", &active);
		selected->SetActive(active);

		ImGui::SameLine();

		// Text Input for the name
		char name[50];
		strcpy_s(name, 50, selected->name.c_str());
		if (ImGui::InputText("", name, 50,
			ImGuiInputTextFlags_AutoSelectAll |
			ImGuiInputTextFlags_EnterReturnsTrue))
			selected->name = name;

		// Transform section
		if (ImGui::CollapsingHeader("Transform"))
		{
			// Position
			float pos[3] = { 1.f,2.f,3.f };
			ImGui::DragFloat3("Position", &selected->transform.M[12]);
		}
	}

    ImGui::End();
}
