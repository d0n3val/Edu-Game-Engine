#include "Globals.h"
#include "Application.h"
#include "PanelResources.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceAudio.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include "ResourceScene.h"
#include "Imgui/imgui.h"
#include <vector>

using namespace std;

// ---------------------------------------------------------
PanelResources::PanelResources() : Panel("Resources", SDL_SCANCODE_F5)
{
	width = 325;
	height = 500;
	posx = 2;
	posy = 500;
}

// ---------------------------------------------------------
PanelResources::~PanelResources()
{}

// ---------------------------------------------------------
void PanelResources::Draw()
{
    ImGui::Begin("Resources", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing );

	static bool waiting_to_load_file = false;

	if (waiting_to_load_file == true && App->editor->FileDialog(nullptr, "/Assets/"))
	{
		const char* file = App->editor->CloseFileDialog();
		if (file != nullptr)
			App->resources->ImportFile(file);
		waiting_to_load_file = false;
	}

	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::MenuItem("Import.."))
			waiting_to_load_file = true;

			// TODO we should safely remove those options
		if (ImGui::MenuItem("Load"))
			App->resources->LoadResources();
		if (ImGui::MenuItem("Save"))
			App->resources->SaveResources();
		ImGui::EndMenu();
	}

	if (ImGui::IsItemHoveredRect() && ImGui::IsMouseClicked(1))
	{
        ImGui::OpenPopup("File Manager");
        if (ImGui::BeginPopup("File Manager"))
        {
			if (ImGui::BeginMenu("Files"))
			{
				LOG("Create new folder");
			}
            ImGui::EndPopup();
        }
	}

	vector<const Resource*> resources;
	// Textures	
	if (ImGui::TreeNodeEx("Textures", 0))
	{
		App->resources->GatherResourceType(resources, Resource::texture);
		for (vector<const Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			const ResourceTexture* info = (const ResourceTexture*)(*it);
			if(ImGui::TreeNodeEx(info->GetExportedFile(), ImGuiTreeNodeFlags_Leaf))
				ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	// Meshes	
	if (ImGui::TreeNodeEx("Meshes", 0))
	{
		App->resources->GatherResourceType(resources, Resource::mesh);
		for (vector<const Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			const ResourceMesh* info = (const ResourceMesh*)(*it);
			if(ImGui::TreeNodeEx(info->GetExportedFile(), ImGuiTreeNodeFlags_Leaf))
				ImGui::TreePop();
		}

		ImGui::TreePop();
	}

	// Audio	
	if (ImGui::TreeNodeEx("Audio", 0))
	{
		App->resources->GatherResourceType(resources, Resource::audio);
		for (vector<const Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			const ResourceAudio* info = (const ResourceAudio*)(*it);
			if (ImGui::TreeNodeEx(info->GetExportedFile(), ImGuiTreeNodeFlags_Leaf))
			{
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	// Scenes	
	if (ImGui::TreeNodeEx("Scenes", 0))
	{
		App->resources->GatherResourceType(resources, Resource::scene);
		for (vector<const Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			const ResourceScene* info = (const ResourceScene*)(*it);
			if (ImGui::TreeNodeEx(info->GetExportedFile(), ImGuiTreeNodeFlags_Leaf))
			{
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
    ImGui::End();
}