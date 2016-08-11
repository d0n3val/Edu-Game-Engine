#include "Globals.h"
#include "Application.h"
#include "PanelGOTree.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleEditor.h"
#include "ModuleCamera3D.h"
#include "GameObject.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
PanelGOTree::PanelGOTree() : Panel("Game Objects Hierarchy", SDL_SCANCODE_F2)
{
	width = 325;
	height = 1002;
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

	if (waiting_to_load_file == true && App->editor->FileDialog("json"))
	{
		const char* file = App->editor->CloseFileDialog();
		if (file != nullptr)
			App->level->Load(file);
		waiting_to_load_file = false;
	}

	if (waiting_to_save_file == true && App->editor->FileDialog("json"))
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

		if(ImGui::MenuItem("Create New"))
			App->level->CreateGameObject();

		if(ImGui::MenuItem("Clear Scene", "!"))
			App->level->RecursiveRemove();

		ImGui::EndMenu();
	}

	// Draw the tree
	GameObject* root = App->level->GetRoot();
	for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
		RecursiveDraw(*it);
    ImGui::End();
}

// ---------------------------------------------------------
void PanelGOTree::RecursiveDraw(const GameObject* go)
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
		if(ImGui::IsItemClicked(0))
			App->editor->selected = (GameObject*) go;

		if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
		{
			float radius = go->global_bbox.MinimalEnclosingSphere().r;
			App->camera->CenterOn(go->GetGlobalPosition(), std::fmaxf(radius, 5.0f) * 3.0f);
		}

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			RecursiveDraw(*it);

		ImGui::TreePop();
	}

	ImGui::PopStyleColor();
}