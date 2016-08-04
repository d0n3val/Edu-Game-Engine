#include "PanelProperties.h"
#include "Imgui/imgui.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"
#include "ModuleMeshes.h"
#include <list>

using namespace std;

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
		if (ImGui::CollapsingHeader("Local Transformation", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float3 pos = selected->GetLocalPosition();
			float3 rot = selected->GetLocalRotation();
			float3 scale = selected->GetLocalScale();;

			if (ImGui::DragFloat3("Position", (float*)&pos, 0.25f))
				selected->SetLocalPosition(pos);

			if(ImGui::SliderFloat3("Rotation", (float*)&rot, -PI, PI))
				selected->SetLocalRotation(rot);

			if (ImGui::DragFloat3("Scale", (float*)&scale, 0.05f))
				selected->SetLocalScale(scale);
		}

		// Iterate all components and draw
		for (list<Component*>::iterator it = selected->components.begin(); it != selected->components.end(); ++it)
		{
			switch ((*it)->GetType())
			{
			case ComponentTypes::Geometry:
				DrawMeshComponent((ComponentMesh*)(*it));
				break;
			case ComponentTypes::Material:
				DrawMaterialComponent((ComponentMaterial*)(*it));
				break;
			case ComponentTypes::AudioSource:
				DrawAudioSourceComponent((ComponentAudioSource*)(*it));
				break;
			case ComponentTypes::AudioListener:
				DrawAudioListenerComponent((ComponentAudioListener*)(*it));
				break;
			default:
				DrawUnknownComponent(*it);
			}
		}

	}

    ImGui::End();
}

void PanelProperties::DrawMeshComponent(ComponentMesh * component)
{
	if (ImGui::CollapsingHeader("Geometry Mesh", ImGuiTreeNodeFlags_DefaultOpen))
	{
		const Mesh* mesh = component->GetMesh();
        ImGui::TextColored(ImVec4(1,1,0,1), "%u Triangles (%u indices %u vertices)",
			mesh->num_indices / 3,
			mesh->num_indices,
			mesh->num_vertices);

		bool uvs = mesh->texture_coords != nullptr;
		bool normals = mesh->normals != nullptr;
		bool colors = mesh->colors != nullptr;

		ImGui::Checkbox("UVs", &uvs);
		ImGui::SameLine();
		ImGui::Checkbox("Normals", &normals);
		ImGui::SameLine();
		ImGui::Checkbox("Colors", &colors);
	}
}

void PanelProperties::DrawAudioSourceComponent(ComponentAudioSource * component)
{
	if (ImGui::CollapsingHeader("Audio Source", ImGuiTreeNodeFlags_DefaultOpen))
	{
	}
}

void PanelProperties::DrawAudioListenerComponent(ComponentAudioListener * component)
{
	if (ImGui::CollapsingHeader("Audio Listener", ImGuiTreeNodeFlags_DefaultOpen))
	{
	}
}

void PanelProperties::DrawMaterialComponent(ComponentMaterial * component)
{
	if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	{
	}
}

void PanelProperties::DrawUnknownComponent(Component * component)
{
	if (ImGui::CollapsingHeader("Unknown", ImGuiTreeNodeFlags_DefaultOpen))
	{
	}
}