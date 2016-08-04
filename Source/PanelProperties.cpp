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
				{
					if(InitComponentDraw(*it, "Geometry Mesh"))
						DrawMeshComponent((ComponentMesh*)(*it));
				}	break;
				case ComponentTypes::Material:
				{
					if(InitComponentDraw(*it, "Material"))
						DrawMaterialComponent((ComponentMaterial*)(*it));
				}	break;
				case ComponentTypes::AudioSource:
				{
					if(InitComponentDraw(*it, "Audio Source"))
						DrawAudioSourceComponent((ComponentAudioSource*)(*it));
				}	break;
				case ComponentTypes::AudioListener:
				{
					if(InitComponentDraw(*it, "Audio Listener"))
						DrawAudioListenerComponent((ComponentAudioListener*)(*it));
				}	break;
				default:
				{
					InitComponentDraw(*it, "Unknown");
				}
			};
		}

	}

    ImGui::End();
}

bool PanelProperties::InitComponentDraw(Component* component, const char * name)
{
	bool ret = false;

	if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool active = component->IsActive();
		if(ImGui::Checkbox("Active", &active))
			component->SetActive(active);
		ret = true;
	}

	return ret;
}

void PanelProperties::DrawMeshComponent(ComponentMesh * component)
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

void PanelProperties::DrawAudioSourceComponent(ComponentAudioSource * component)
{
	const char* file = component->GetFile();

	ImGui::Text("File: ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1, 1, 0, 1), (file) ? file : "No file loaded");
	ImGui::SameLine();
	ImGui::Checkbox("Is 2D", &component->is_2d);

	ImGui::SliderFloat("Fade In", (float*)&component->fade_in, 0.0f, 10.0f);
	ImGui::SliderFloat("Fade Out", (float*)&component->fade_out, 0.0f, 10.0f);
	ImGui::DragFloat("Min Distance", (float*)&component->min_distance, 0.1f, 0.1f, 10000.0f);
	ImGui::DragFloat("Max Distance", (float*)&component->max_distance, 0.1f, 0.1f, 10000.0f);
	ImGui::SliderInt("Cone In", (int*)&component->cone_angle_in, 0, 360);
	ImGui::SliderInt("Cone Out", (int*)&component->cone_angle_out, 0, 360);
	ImGui::SliderFloat("Vol Out Cone", (float*)&component->out_cone_vol, 0.0f, 1.0f);
	
	static const char * states[] = { 
		"Not Loaded", 
		"Stopped",
		"About to Play",
		"Playing",
		"About to Pause",
		"Pause",
		"About to Unpause",
		"About to Stop"
	};

	ImGui::Text("Current State: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", states[component->GetCurrentState()]);

	if (ImGui::Button("Play"))
		component->Play();

	ImGui::SameLine();
	if (ImGui::Button("Pause"))
		component->Pause();

	ImGui::SameLine();
	if (ImGui::Button("Unpause"))
		component->UnPause();

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		component->Stop();
}

void PanelProperties::DrawAudioListenerComponent(ComponentAudioListener * component)
{
	ImGui::DragFloat("Distance", (float*)&component->distance, 0.1f, 0.1f, 10000.0f);
	ImGui::SliderFloat("Roll Off", (float*)&component->roll_off, 0.0f, 10.0f);
	ImGui::SliderFloat("Doppler", (float*)&component->doppler, 0.0f, 10.0f);
}

void PanelProperties::DrawMaterialComponent(ComponentMaterial * component)
{
}
