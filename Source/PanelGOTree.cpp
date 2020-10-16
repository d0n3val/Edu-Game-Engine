#include "Globals.h"
#include "Application.h"
#include "PanelGOTree.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleEditor.h"
#include "ModuleEditorCamera.h"
#include "ModuleResources.h"
#include "ResourceModel.h"
#include "GameObject.h"

#include <list>

#include <stdio.h>

#include "Leaks.h"

using namespace std;

// ---------------------------------------------------------
PanelGOTree::PanelGOTree() : Panel("Game Objects")
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
	//ImGui::SetNextWindowContentWidth((float) (width*2));
    //ImGui::Begin("GameObjects Hierarchy", &active, 
		//ImGuiWindowFlags_NoResize | 
		//ImGuiWindowFlags_NoFocusOnAppearing |
		//ImGuiWindowFlags_HorizontalScrollbar );

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
		bool sel = false;
		if (ImGui::MenuItem("Load..", "", &sel, App->IsStop()))
			waiting_to_load_file = true;

		if (ImGui::MenuItem("Save..", "", &sel, App->IsStop()))
			waiting_to_save_file = true;

		if (ImGui::BeginMenu("Load Prefab"))
		{
			if (ImGui::BeginMenu("Model"))
			{
				vector<const Resource*> resources;
				App->resources->GatherResourceType(resources, Resource::model);

				for (vector<const Resource*>::const_iterator it = resources.begin(); it != resources.end(); ++it)
				{
					const Resource* model = (*it);
                    ImGui::PushID(model->GetExportedFile());
					if (ImGui::MenuItem(model->GetSourceName()))
					{
                        App->level->AddModel(model->GetUID());
					}
                    ImGui::PopID();
				}

				ImGui::EndMenu();
			}
            ImGui::EndMenu();
		}

		if(ImGui::MenuItem("New Game Object"))
			App->level->CreateGameObject();

        if(ImGui::MenuItem("New Point Light"))
            App->level->AddPointLight();

        if(ImGui::MenuItem("New Spot Light"))
            App->level->AddSpotLight();

		if (ImGui::MenuItem("Clear Scene", "!"))
			App->level->GetRoot()->Remove();


		ImGui::EndMenu();
	}

    ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_WHITE);

    if(ImGui::TreeNodeEx("GameObjecs", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // Draw the tree
        GameObject* root = App->level->GetRoot();
        for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
            RecursiveDraw(*it);

        ImGui::TreePop();
    }

    ImGui::PopStyleColor();

    DrawLights();

	if (drag && ImGui::IsMouseReleased(0))
		drag = nullptr;

    //ImGui::End();
}

// ---------------------------------------------------------
void PanelGOTree::DrawLights()
{
    ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_WHITE);

    if(ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_GREY);
        uint flags = ImGuiTreeNodeFlags_Leaf;

        if(App->editor->selection_type == ModuleEditor::SelectionAmbientLight && App->editor->selected.ambient == App->level->GetAmbientLight())
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if(ImGui::TreeNodeEx("Ambient", flags))
        {
            if (ImGui::IsItemClicked(0)) 
            {
                App->editor->selected.ambient = App->level->GetAmbientLight();
                App->editor->selection_type = ModuleEditor::SelectionAmbientLight;
            }
            ImGui::TreePop();
        }

        flags = ImGuiTreeNodeFlags_Leaf;

        if(App->editor->selection_type == ModuleEditor::SelectionDirLight && App->editor->selected.directional == App->level->GetDirLight())
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if(ImGui::TreeNodeEx("Directional", flags))
        {
            if (ImGui::IsItemClicked(0)) 
            {
                App->editor->selected.directional = App->level->GetDirLight();
                App->editor->selection_type = ModuleEditor::SelectionDirLight;
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNodeEx("Point", 0))
        {
            bool remove = false;
            char number[16];
            for(uint i=0, count = App->level->GetNumPointLights(); !remove && i < count; ++i)
            {
                sprintf_s(number, 15, "[%d]", i);

                flags = ImGuiTreeNodeFlags_Leaf;

                if(App->editor->selection_type == ModuleEditor::SelectionPointLight && App->editor->selected.point == App->level->GetPointLight(i))
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if(ImGui::TreeNodeEx(number, flags))
                {
                    if (ImGui::IsItemClicked(0)) 
                    {
                        App->editor->selected.point = App->level->GetPointLight(i);
                        App->editor->selection_type = ModuleEditor::SelectionPointLight;
                    }

                    if (ImGui::IsItemClicked(1))
                        ImGui::OpenPopup("PointLight Options");

                    if (ImGui::BeginPopup("PointLight Options"))
                    {
                        if (true == (remove = ImGui::MenuItem("Remove")))
                        {
                            if(App->editor->selection_type == ModuleEditor::SelectionPointLight && 
                               App->editor->selected.point == App->level->GetPointLight(i))
                            {
                                App->editor->selected.point = nullptr;
                            }

                            App->level->RemovePointLight(i);
                        }
                        ImGui::EndPopup();
                    }


                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        if(ImGui::TreeNodeEx("Spot", 0))
        {
            bool remove = false;
            char number[16];
            for(uint i=0, count = App->level->GetNumSpotLights(); !remove && i < count; ++i)
            {
                sprintf_s(number, 15, "[%d]", i);

                flags = ImGuiTreeNodeFlags_Leaf;

                if(App->editor->selection_type == ModuleEditor::SelectionSpotLight && App->editor->selected.spot == App->level->GetSpotLight(i))
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if(ImGui::TreeNodeEx(number, flags))
                {
                    if (ImGui::IsItemClicked(0)) 
                    {
                        App->editor->selected.spot = App->level->GetSpotLight(i);
                        App->editor->selection_type = ModuleEditor::SelectionSpotLight;
                    }

                    if (ImGui::IsItemClicked(1))
                        ImGui::OpenPopup("SpotLight Options");

                    if (ImGui::BeginPopup("SpotLight Options"))
                    {
                        if (true == (remove = ImGui::MenuItem("Remove")))
                        {
                            if(App->editor->selection_type == ModuleEditor::SelectionSpotLight && 
                               App->editor->selected.spot == App->level->GetSpotLight(i))
                            {
                                App->editor->selected.spot = nullptr;
                            }

                            App->level->RemoveSpotLight(i);
                        }
                        ImGui::EndPopup();
                    }


                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
        ImGui::TreePop();
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor();
}

// ---------------------------------------------------------
void PanelGOTree::RecursiveDraw(GameObject* go)
{
	sprintf_s(name, 80, "%s##node_%i", go->name.empty() ? "(empty)": go->name.c_str(), node++);
	uint flags = 0;// ImGuiTreeNodeFlags_OpenOnArrow;

	if (go->childs.size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	bool go_selection = App->editor->selection_type == ModuleEditor::SelectionGameObject;
	if (go_selection && go == App->editor->selected.go) 
	{
		flags |= ImGuiTreeNodeFlags_Selected;
		open_selected = false;
	}

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

	if (open_selected == true && go_selection == true && App->editor->selected.go->IsUnder(go) == true)
		ImGui::SetNextTreeNodeOpen(true);

	if (ImGui::TreeNodeEx(name, flags))
	{
		CheckHover(go);

		if (ImGui::IsItemClicked(0)) {
			App->editor->selected.go = go;
			App->editor->selection_type = ModuleEditor::SelectionGameObject;
			drag = go;
		}

        if (ImGui::BeginPopupContextItem())
        {
			if (ImGui::MenuItem("Duplicate"))
				App->level->Duplicate(go);
			if (ImGui::MenuItem("Remove"))
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
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
	{
		if (drag && drag != go)
		{
			ImGui::BeginTooltip();
			ImGui::Text("Move %s to %s", drag->name.c_str(), go->name.c_str());
			ImGui::EndTooltip();
		}

		if (ImGui::IsMouseClicked(0)) {
			App->editor->selection_type = ModuleEditor::SelectionGameObject;
			App->editor->selected.go = go;
			drag = go;
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

