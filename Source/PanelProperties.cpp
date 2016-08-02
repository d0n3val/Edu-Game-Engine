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

		// Transform section ============================================
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			aiVector3D pos = selected->GetLocalPosition();
			aiVector3D rot = selected->GetLocalRotation();
			aiVector3D scale = selected->GetLocalScale();;

			if (ImGui::DragFloat3("Position", (float*)&pos, 0.25f))
				selected->SetLocalPosition(pos);

			if(ImGui::SliderFloat3("Rotation", (float*)&rot, -PI, PI))
				selected->SetLocalRotation(rot);

			if (ImGui::DragFloat3("Scale", (float*)&scale, 0.05f))
				selected->SetLocalScale(scale);
		}
	}

    ImGui::End();
}
