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
PanelResources::PanelResources() : Panel("Resources")
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
			App->resources->ImportFile(file, true); // \todo: when is new generates an instance???
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

	DrawResourceType(Resource::model);
	DrawResourceType(Resource::material);
	DrawResourceType(Resource::texture);
	DrawResourceType(Resource::mesh);
	DrawResourceType(Resource::audio);
	DrawResourceType(Resource::scene);
	DrawResourceType(Resource::bone);
	DrawResourceType(Resource::animation);

    ImGui::End();
}


UID PanelResources::DrawResourceType(Resource::Type type)
{
	UID selected = 0;
	vector<const Resource*> resources;

	static const char* titles[] = {
		"Models", "Materials", "Textures", "Meshes", "Audios", "Scenes", "Animation", "Others" };

	if (ImGui::TreeNodeEx(titles[type], 0))
	{
		App->resources->GatherResourceType(resources, type);
		for (vector<const Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			const Resource* info = (*it);
			if (ImGui::TreeNodeEx(info->GetExportedFile(), ImGuiTreeNodeFlags_Leaf))
			{
				if (ImGui::IsItemClicked())
					selected = info->GetUID();

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::Text("UID: %llu", info->GetUID());
					ImGui::Text("Source: %s", info->GetFile());
					ImGui::Text("References: %u", info->CountReferences());
					ImGui::EndTooltip();
				}

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}
	return selected;
}
