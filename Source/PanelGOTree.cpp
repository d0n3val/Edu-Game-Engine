#include "PanelGOTree.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "ModuleScene.h"
#include "GameObject.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
PanelGOTree::PanelGOTree() : Panel("Game Objects Hierarchy", SDL_SCANCODE_F2)
{}

// ---------------------------------------------------------
PanelGOTree::~PanelGOTree()
{}

// ---------------------------------------------------------
void PanelGOTree::Draw()
{
	node = 0;
    ImGui::Begin("GameObjects Hierarchy", &active, ImGuiWindowFlags_NoFocusOnAppearing );

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

	if(go->IsActive() == false)
		ImGui::PushStyleColor(ImGuiCol_Text, ImColor::HSV(7.0f, 0.6f, 0.6f));

	if (ImGui::TreeNodeEx(name, flags))
	{
		if (ImGui::IsItemClicked())
			selected = go;

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			RecursiveDraw(*it);

		ImGui::TreePop();
	}

	if(go->IsActive() == false)
		ImGui::PopStyleColor(1);
}