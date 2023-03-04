#include "Globals.h"
#include "Application.h"
#include "PanelGOTree.h"
#include "Imgui/imgui.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "LightManager.h"
#include "ModuleEditor.h"
#include "ModuleEditorCamera.h"
#include "ModuleResources.h"
#include "ResourceModel.h"
#include "GameObject.h"

#include <list>
#include <variant>

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
					if (ImGui::MenuItem(model->GetUserResName()))
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
            App->level->GetLightManager()->AddPointLight();

        if(ImGui::MenuItem("New Spot Light"))
            App->level->GetLightManager()->AddSpotLight();

        if(ImGui::MenuItem("New Quad Light"))
            App->level->GetLightManager()->AddQuadLight();

        if(ImGui::MenuItem("New Sphere Light"))
            App->level->GetLightManager()->AddSphereLight();


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
            if (RecursiveDraw(*it) == true)
                break;

        ImGui::TreePop();
    }

    ImGui::PopStyleColor();

    DrawLights();
    DrawSkybox();

	if (drag && ImGui::IsMouseReleased(0))
		drag = nullptr;

    //ImGui::End();
}

// ---------------------------------------------------------
void PanelGOTree::DrawSkybox()
{
    Skybox* const* skybox = std::get_if<Skybox*>(&App->editor->GetSelection());

    uint flags = ImGuiTreeNodeFlags_Leaf;
    if(skybox != nullptr && *skybox == App->level->GetSkyBox())
    {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    if(ImGui::TreeNodeEx("Skybox", flags))
    {
        if (ImGui::IsItemClicked(0)) 
        {
            App->editor->SetSelected(App->level->GetSkyBox());
        }

        ImGui::TreePop();
    }

}

// ---------------------------------------------------------
void PanelGOTree::DrawLights()
{
    ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_WHITE);

    if(ImGui::TreeNodeEx("Lights", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_GREY);
        uint flags = ImGuiTreeNodeFlags_Leaf;

        DirLight* const* dir_light = std::get_if<DirLight*>(&App->editor->GetSelection());

        if(dir_light && *dir_light == App->level->GetLightManager()->GetDirLight())
        {
            flags |= ImGuiTreeNodeFlags_Selected;
        }

        if(ImGui::TreeNodeEx("Directional", flags))
        {
            if (ImGui::IsItemClicked(0)) 
            {
                App->editor->SetSelected(App->level->GetLightManager()->GetDirLight());
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNodeEx("Point", 0))
        {
            bool remove = false;
            char number[16];
            for(uint i=0, count = App->level->GetLightManager()->GetNumPointLights(); !remove && i < count; ++i)
            {
                sprintf_s(number, 15, "[%d]", i);

                flags = ImGuiTreeNodeFlags_Leaf;

                PointLight* const* point_light = std::get_if<PointLight*>(&App->editor->GetSelection());
                bool is_selected = point_light && *point_light == App->level->GetLightManager()->GetPointLight(i);
                if(is_selected)
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if(ImGui::TreeNodeEx(number, flags))
                {
                    if (ImGui::IsItemClicked(0)) 
                    {
                        App->editor->SetSelected(App->level->GetLightManager()->GetPointLight(i));
                    }

                    if (ImGui::IsItemClicked(1))
                        ImGui::OpenPopup("PointLight Options");

                    if (ImGui::BeginPopup("PointLight Options"))
                    {
                        if (true == (remove = ImGui::MenuItem("Remove")))
                        {
                            if(is_selected)
                            {
                                App->editor->ClearSelected();                                
                            }

                            App->level->GetLightManager()->RemovePointLight(i);
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

            SpotLight* const* spot = std::get_if<SpotLight*>(&App->editor->GetSelection());

            for(uint i=0, count = App->level->GetLightManager()->GetNumSpotLights(); !remove && i < count; ++i)
            {
                sprintf_s(number, 15, "[%d]", i);

                flags = ImGuiTreeNodeFlags_Leaf;

                if(spot && *spot == App->level->GetLightManager()->GetSpotLight(i))
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if(ImGui::TreeNodeEx(number, flags))
                {
                    if (ImGui::IsItemClicked(0)) 
                    {
                        App->editor->SetSelected(App->level->GetLightManager()->GetSpotLight(i));
                    }

                    if (ImGui::IsItemClicked(1))
                        ImGui::OpenPopup("SpotLight Options");

                    if (ImGui::BeginPopup("SpotLight Options"))
                    {
                        if (true == (remove = ImGui::MenuItem("Remove")))
                        {
                            if(spot && *spot == App->level->GetLightManager()->GetSpotLight(i))
                            {
                                App->editor->ClearSelected();
                            }

                            App->level->GetLightManager()->RemoveSpotLight(i);
                        }
                        ImGui::EndPopup();
                    }


                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        if(ImGui::TreeNodeEx("Quad", 0))
        {
            bool remove = false;
            char number[16];

            QuadLight* const* quad = std::get_if<QuadLight*>(&App->editor->GetSelection());

            for(uint i=0, count = App->level->GetLightManager()->GetNumQuadLights(); !remove && i < count; ++i)
            {
                sprintf_s(number, 15, "[%d]", i);

                flags = ImGuiTreeNodeFlags_Leaf;

                if(quad && *quad == App->level->GetLightManager()->GetQuadLight(i))
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if(ImGui::TreeNodeEx(number, flags))
                {
                    if (ImGui::IsItemClicked(0)) 
                    {
                        App->editor->SetSelected(App->level->GetLightManager()->GetQuadLight(i));
                    }

                    if (ImGui::IsItemClicked(1))
                        ImGui::OpenPopup("QuadLight Options");

                    if (ImGui::BeginPopup("QuadLight Options"))
                    {
                        if (true == (remove = ImGui::MenuItem("Remove")))
                        {
                            if(quad && *quad == App->level->GetLightManager()->GetQuadLight(i))
                            {
                                App->editor->ClearSelected();
                            }

                            App->level->GetLightManager()->RemoveQuadLight(i);
                        }
                        ImGui::EndPopup();
                    }


                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }

        if(ImGui::TreeNodeEx("Sphere", 0))
        {
            bool remove = false;
            char number[16];

            SphereLight* const* sphere = std::get_if<SphereLight*>(&App->editor->GetSelection());

            for(uint i=0, count = App->level->GetLightManager()->GetNumSphereLights(); !remove && i < count; ++i)
            {
                sprintf_s(number, 15, "[%d]", i);

                flags = ImGuiTreeNodeFlags_Leaf;

                if(sphere && *sphere == App->level->GetLightManager()->GetSphereLight(i))
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                if(ImGui::TreeNodeEx(number, flags))
                {
                    if (ImGui::IsItemClicked(0)) 
                    {
                        App->editor->SetSelected(App->level->GetLightManager()->GetSphereLight(i));
                    }

                    if (ImGui::IsItemClicked(1))
                        ImGui::OpenPopup("SphereLight Options");

                    if (ImGui::BeginPopup("SphereLight Options"))
                    {
                        if (true == (remove = ImGui::MenuItem("Remove")))
                        {
                            if(sphere && *sphere == App->level->GetLightManager()->GetSphereLight(i))
                            {
                                App->editor->ClearSelected();
                            }

                            App->level->GetLightManager()->RemoveSphereLight(i);
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
bool PanelGOTree::RecursiveDraw(GameObject* go)
{
    bool stop = false;
	sprintf_s(name, 80, "%s##node_%i", go->name.empty() ? "(empty)": go->name.c_str(), node++);
	uint flags = 0;// ImGuiTreeNodeFlags_OpenOnArrow;

    const char* str = strstr(go->name.c_str(), "$AssimpFbx$");
    if (str != nullptr)
    {
        for (GameObject* go : go->childs) 
        {
            if((stop = RecursiveDraw(go)) == true)
            {
                break;
            }
        }
    }
    else
    {
        if (go->childs.size() == 0)
            flags |= ImGuiTreeNodeFlags_Leaf;


        GameObject* const* selected_go = std::get_if<GameObject*>(&App->editor->GetSelection());

        if (selected_go && go == *selected_go)
        {
            flags |= ImGuiTreeNodeFlags_Selected;
            open_selected = false;
        }

        ImVec4 color = IMGUI_WHITE;

        if (go->IsActive() == false)
            color = IMGUI_RED;

        if (go->visible == false)
            color = IMGUI_GREY;

        if (go->WasBBoxDirty() == true)
            color = IMGUI_GREEN;

        if (go->WasDirty() == true)
            color = IMGUI_YELLOW;

        ImGui::PushStyleColor(ImGuiCol_Text, color);

        if (open_selected == true && selected_go && (*selected_go)->IsUnder(go) == true)
            ImGui::SetNextTreeNodeOpen(true);

        if (ImGui::TreeNodeEx(name, flags))
        {
            CheckHover(go);

            if (ImGui::IsItemClicked(0)) {
                App->editor->SetSelected(go);
                drag = go;
            }

            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Duplicate"))
                    App->level->Duplicate(go);
                if (ImGui::MenuItem("Remove"))
                {
                    App->editor->ClearSelected();
                    drag = nullptr;
                    go->Remove();
                    stop = true;
                }

                ImGui::EndPopup();
            }

            if(!stop)
            {
                for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
                    if ((stop = RecursiveDraw(*it)) == true) break;
            }

            ImGui::TreePop();
        }
        else
            CheckHover(go);

        ImGui::PopStyleColor();
    }

    return stop;
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
            App->editor->SetSelected(go);
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

