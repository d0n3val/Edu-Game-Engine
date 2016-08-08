#include "PanelGOTree.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "ModuleScene.h"
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

	GameObject* root = App->scene->GetRoot();
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
	if (go == selected)
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
		if (ImGui::IsItemClicked())
			selected = go;

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			RecursiveDraw(*it);

		ImGui::TreePop();
	}

	ImGui::PopStyleColor();
}