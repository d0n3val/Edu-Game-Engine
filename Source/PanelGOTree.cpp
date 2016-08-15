#include "Globals.h"
#include "Application.h"
#include "PanelGOTree.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleEditor.h"
#include "ModuleCamera3D.h"
#include "ModuleResources.h"
#include "GameObject.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
PanelGOTree::PanelGOTree() : Panel("Game Objects Hierarchy", SDL_SCANCODE_F2)
{
	width = 325;
	height = 500;
	posx = 2;
	posy = 21;
}

// ---------------------------------------------------------
PanelGOTree::~PanelGOTree()
{}

// ---------------------------------------------------------
void PanelGOTree::Draw()
{
	node = 0;
    ImGui::Begin("GameObjects Hierarchy", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing );

	// Menu ---
	static bool waiting_to_load_file = false;
	static bool waiting_to_save_file = false;

	if (waiting_to_load_file == true && App->editor->FileDialog("eduscene"))
	{
		const char* file = App->editor->CloseFileDialog();
		if (file != nullptr)
			App->level->Load(file);
		waiting_to_load_file = false;
	}

	if (waiting_to_save_file == true && App->editor->FileDialog("eduscene"))
	{
		const char* file = App->editor->CloseFileDialog();
		if (file != nullptr)
			App->level->Save(file);
		waiting_to_save_file = false;
	}

	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::MenuItem("Load.."))
			waiting_to_load_file = true;

		if (ImGui::MenuItem("Save.."))
			waiting_to_save_file = true;

		if (ImGui::BeginMenu("Load Prefab"))
		{
			vector<const Resource*> resources;
			App->resources->GatherResourceType(resources, Resource::scene);

			for (vector<const Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
			{
				const Resource* info = (*it);
				if (ImGui::MenuItem(info->GetExportedFile()))
				{
					string file(LIBRARY_SCENE_FOLDER);
					file += info->GetExportedFile();
					App->level->Load(file.c_str());
				}
			}
            ImGui::EndMenu();
		}

		if(ImGui::MenuItem("Create New"))
			App->level->CreateGameObject();

		if (ImGui::MenuItem("Clear Scene", "!"))
			App->level->GetRoot()->Remove();

		ImGui::EndMenu();
	}

	// Draw the tree
	GameObject* root = App->level->GetRoot();
	for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
		RecursiveDraw(*it);

	if (drag && ImGui::IsMouseReleased(0))
	{
		LOG("Mouse drag stopped somewhere else");
		drag = nullptr;
	}

    ImGui::End();
}

// ---------------------------------------------------------
void PanelGOTree::RecursiveDraw(GameObject* go)
{
	sprintf_s(name, 80, "%s##node_%i", go->name.c_str(), node++);
	uint flags = 0;// ImGuiTreeNodeFlags_OpenOnArrow;

	if (go->childs.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (go == App->editor->selected)
		flags |= ImGuiTreeNodeFlags_Selected;

	ImVec4 color = IMGUI_WHITE;

	if(go->IsActive() == false)
		color = IMGUI_RED;

	if (go->visible == false)
		color = IMGUI_GREY;

	if (go->WasBBoxDirty() == true)
		color = IMGUI_GREEN;

	if (go->WasDirty() == true)
		color = IMGUI_YELLOW;

	ImGui::PushStyleColor(ImGuiCol_Text, color);

	if (ImGui::TreeNodeEx(name, flags))
	{
		CheckHover(go);

		if (ImGui::IsItemClicked(0)) {
			App->editor->selected = go;
			drag = App->editor->selected;
		}

		if (ImGui::IsItemClicked(1))
			ImGui::OpenPopup("GameObject Options");

		if (ImGui::BeginPopup("GameObject Options"))
		{
			if (ImGui::MenuItem("Delete"))
				go->Remove();
			ImGui::EndPopup();
		}

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			RecursiveDraw(*it);

		ImGui::TreePop();
	}
	else
		CheckHover(go);

	ImGui::PopStyleColor();
}

void PanelGOTree::CheckHover(GameObject* go)
{
	if (ImGui::IsItemHoveredRect())
	{
		if (drag && drag != go)
		{
			ImGui::BeginTooltip();
			ImGui::Text("Move %s to %s", drag->name.c_str(), go->name.c_str());
			ImGui::EndTooltip();
		}

		if (ImGui::IsMouseClicked(0)) {
			App->editor->selected = go;
			drag = App->editor->selected;
		}

		if (ImGui::IsMouseDoubleClicked(0))
		{
			float radius = go->global_bbox.MinimalEnclosingSphere().r;
			App->camera->CenterOn(go->GetGlobalPosition(), std::fmaxf(radius, 5.0f) * 2.0f);
		}

		if (drag && ImGui::IsMouseReleased(0) && drag != go)
		{
			drag->SetNewParent(go, true);
			drag = nullptr;
		}
	}
}

