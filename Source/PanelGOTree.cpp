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
	uint flags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (go->childs.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;
	if (ImGui::TreeNodeEx(name, flags))
	{
		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			RecursiveDraw(*it);
		ImGui::TreePop();
	}
}