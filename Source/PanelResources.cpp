#include "Globals.h"
#include "Application.h"
#include "PanelResources.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "ResourceAudio.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"
#include <vector>
#include <filesystem>

#include "Leaks.h"

using namespace std;

// ---------------------------------------------------------
PanelResources::PanelResources() : Panel("Resources")
{
	width = 325;
	height = 500;
	posx = 2;
	posy = 500;

    fileDialog.SetTitle("Import resource");
}

// ---------------------------------------------------------
PanelResources::~PanelResources()
{}

// ---------------------------------------------------------
void PanelResources::Draw()
{
    fileDialog.Display();
	if(fileDialog.HasSelected())
    {
        ImportResource(std::filesystem::relative(fileDialog.GetSelected(), std::filesystem::current_path()).generic_string());
        fileDialog.ClearSelected();
    }

    textures_dlg.Display();
    if(textures_dlg.HasSelection())
    {
        App->resources->ImportTexture(textures_dlg.GetFile().c_str(), textures_dlg.GetCompressed(), textures_dlg.GetMipmaps(), textures_dlg.GetToCubemap()); 
        textures_dlg.ClearSelection();
    }

    animation_dlg.Display();
    if(animation_dlg.HasSelection())
    {
        for(const ImportAnimationDlg::Clip& clip : animation_dlg.GetClips())
        {
            App->resources->ImportAnimation(animation_dlg.GetFile().c_str(), clip.first, clip.last, clip.name, animation_dlg.GetScale()); 
        }
        animation_dlg.ClearSelection();
    }

    cubemap_dlg.Display();
    if(cubemap_dlg.HasSelection())
    {
        App->resources->ImportCubemap(cubemap_dlg.GetFiles(), std::string(), cubemap_dlg.GetCompressed(), cubemap_dlg.GetMipmaps());
        cubemap_dlg.ClearSelection();
    }

    model_dlg.Display();
    if(model_dlg.HasSelection())
    {
        App->resources->ImportModel(model_dlg.GetFile().c_str(), model_dlg.GetScale(), model_dlg.GetUserName().c_str());
        model_dlg.ClearSelection();
    }

    show_texture.Display();

	if (ImGui::BeginMenu("Options"))
	{
			// TODO we should safely remove those options
		if (ImGui::MenuItem("Load"))
			App->resources->LoadResources();
		if (ImGui::MenuItem("Save"))
			App->resources->SaveResources();
			//App->resources->SaveResourcesTo("test");
		ImGui::EndMenu();
	}

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly) && ImGui::IsMouseClicked(1))
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

	DrawResourceType(Resource::model, &PanelResources::DrawResourcePopup);
	DrawResourceType(Resource::material, &PanelResources::DrawResourcePopup);
	DrawResourceType(Resource::texture, &PanelResources::DrawResourcePopup);

	DrawResourceType(Resource::mesh, &PanelResources::DrawMeshPopup);

    DrawResourceType(Resource::audio, &PanelResources::DrawResourcePopup);
    DrawResourceType(Resource::animation, &PanelResources::DrawResourcePopup);
    DrawResourceType(Resource::state_machine, &PanelResources::DrawResourcePopup);

    //ImGui::End();
}

void PanelResources::ImportResource(const std::string& file)
{
    if (!file.empty())
    {
        switch(waiting_to_load)
        {
            case Resource::model:
                {
                    model_dlg.Open(file);
                    break;
                }
            case Resource::texture:
                {
                    textures_dlg.Open(file);
                    break;
                }
            case Resource::animation:
                {
                    std::string user_name;
                    App->fs->SplitFilePath(file.c_str(), nullptr, &user_name, nullptr);

                    size_t pos_dot = user_name.find_last_of(".");
                    if(pos_dot != std::string::npos)
                    {
                        user_name.erase(user_name.begin()+pos_dot, user_name.end());
                    }

                    animation_dlg.Open(file, user_name);
                    break;
                }
            default:
                App->resources->ImportFile(file.c_str(), waiting_to_load, false); 
                break;
        }
    }
    waiting_to_load = Resource::unknown;
}

void PanelResources::DrawResourceType(Resource::Type type, void (PanelResources::*popup)(Resource::Type))
{
	vector<const Resource*> resources;

	static const char* titles[] = {
		"Models", "Materials", "Textures", "Meshes", "Audios", "Animation", "State machines", "Others" };

    bool open_tree =ImGui::TreeNodeEx(titles[type], 0);

	if(popup != nullptr)
    {
        if (ImGui::IsItemClicked(1))
        {
			ImGui::PushID(Resource::GetTypeStr(type));
            ImGui::OpenPopup("Resource popup");
			ImGui::PopID();
        }

        (this->*popup)(type);
    }

    if (open_tree)
    {
        bool append_selection = !selection.empty() && (App->input->GetKey(SDL_SCANCODE_LCTRL) || App->input->GetKey(SDL_SCANCODE_RCTRL));
        bool multiple_select = App->input->GetKey(SDL_SCANCODE_LSHIFT) || App->input->GetKey(SDL_SCANCODE_RSHIFT);
        uint pivot = multiple_select_type == type ? multiple_select_pivot : 0;

        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_LIGHT_GREY);
        bool remove = false;
        App->resources->GatherResourceType(resources, type);
        for (vector<const Resource*>::const_iterator it = resources.begin(); !remove && it != resources.end(); ++it)
        {
            const Resource* info = (*it);

			bool selected = selection.find(info->GetUID()) != selection.end();

            const char* lName = info->GetUserResName();
            if (strlen(lName) == 0)
            {
                lName = "*NoName*";
            }

            ImGui::PushID(info->GetExportedFile());
            if (ImGui::TreeNodeEx(lName, selected ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_Leaf))
            {
                if (ImGui::IsItemClicked(0))
                {
                    ManageSelection(resources, uint(it-resources.begin()), append_selection, multiple_select, pivot, false);
                }
                
				if(ImGui::IsItemClicked(1))
                {
                    ManageSelection(resources, uint(it-resources.begin()), append_selection, multiple_select, pivot, true);
					
					if(ImGui::IsItemHovered())
						ImGui::OpenPopup("Item popup");
				}

				if (ImGui::BeginPopup("Item popup"))
				{
					if (true == (remove = ImGui::MenuItem("Remove")))
					{
						for (std::set<UID>::const_iterator selected_it = selection.begin(), selected_end = selection.end(); selected_it != selected_end; ++selected_it)
						{
							App->resources->RemoveResource(*selected_it);
						}

						multiple_select_type = Resource::unknown;
						multiple_select_pivot = 0;
						selection.clear();
					}
                    else if(type == Resource::texture && !selection.empty() && ImGui::MenuItem("Show"))
                    {
                        ResourceTexture* texture = static_cast<ResourceTexture*>(App->resources->Get(*selection.rbegin()));
                        show_texture.Open(nullptr, texture);
                    }

					ImGui::EndPopup();
				}
                else if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("UID: %llu", info->GetUID());
                    ImGui::Text("Source: %s", info->GetFile());
                    ImGui::Text("Exported: %s", info->GetExportedFile());
                    ImGui::Text("References: %u", info->CountReferences());
                    ImGui::EndTooltip();
                }

                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::TreePop();
        ImGui::PopStyleColor();
    }
}

void PanelResources::ManageSelection(const std::vector<const Resource*>& resources, uint current, bool append, bool multiple, uint pivot, bool popup)
{
    if (append)
    {
        if(multiple)
        {
            if(pivot > current)
            {
                for(vector<const Resource*>::const_iterator multiple_it = resources.begin()+current, multipole_end = resources.begin()+pivot+1; 
                        multiple_it != multipole_end; ++multiple_it)
                {
                    selection.insert((*multiple_it)->GetUID());
                }
            }
            else
            {
                for(vector<const Resource*>::const_iterator multiple_it = resources.begin()+pivot, multipole_end = resources.begin()+current+1; 
                        multiple_it != multipole_end; ++multiple_it)
                {
                    selection.insert((*multiple_it)->GetUID());
                }
            }
        }
        else
        {
            std::set<UID>::iterator it = selection.find(resources[current]->GetUID());
            if(it == selection.end())
            {
                selection.insert(resources[current]->GetUID());
            }
            else
            {
                selection.erase(it);
            }
        }
    }
    else
    {
        std::set<UID>::iterator it = selection.find(resources[current]->GetUID());
        if(!popup || it == selection.end())
        {
            selection.clear();
        }

        if(!popup && multiple)
        {
            if(pivot > current)
            {
                for(vector<const Resource*>::const_iterator multiple_it = resources.begin()+current, multipole_end = resources.begin()+pivot+1; 
                        multiple_it != multipole_end; ++multiple_it)
                {
                    selection.insert((*multiple_it)->GetUID());
                }
            }
            else
            {
                for(vector<const Resource*>::const_iterator multiple_it = resources.begin()+pivot, multipole_end = resources.begin()+current+1; 
                        multiple_it != multipole_end; ++multiple_it)
                {
                    selection.insert((*multiple_it)->GetUID());
                }
            }
        }
        else
        {
            selection.insert(resources[current]->GetUID());
            multiple_select_type = resources[current]->GetType();
            multiple_select_pivot = current;
        }
    }
}

void PanelResources::DrawResourcePopup(Resource::Type type)
{
    ImGui::PushID(Resource::GetTypeStr(type));
    if(ImGui::BeginPopup("Resource popup"))
    {
		if (ImGui::MenuItem("Import.."))
        {
            waiting_to_load = type;
            fileDialog.SetPwd(std::filesystem::path("Assets"));
            fileDialog.Open();
        }

        if(type == Resource::texture && ImGui::MenuItem("Import Cubemap.."))
        {
            waiting_to_load = type;
            cubemap_dlg.Open();
        }

        if(type == Resource::model && ImGui::MenuItem("Force save"))
        {
            App->resources->SaveTypedResources(type);
        }

        ImGui::EndPopup();
    }
    ImGui::PopID();
}

void PanelResources::DrawMeshPopup(Resource::Type type)
{
    bool open_plane     = false;
    bool open_cylinder  = false;
    bool open_sphere    = false;

    ImGui::PushID(Resource::GetTypeStr(type));
    if(ImGui::BeginPopup("Resource popup"))
    {
		if (ImGui::MenuItem("Import.."))
        {
            waiting_to_load = type;
        }

        if(ImGui::MenuItem("Force save"))
        {
            App->resources->SaveTypedResources(type);
        }

        if (ImGui::BeginMenu("Add prefab"))
        {
            open_plane = ImGui::MenuItem("Plane");
            open_cylinder = ImGui::MenuItem("Cylinder");
            open_sphere = ImGui::MenuItem("Sphere");
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }
    ImGui::PopID();

    if(open_plane)
    {
        ImGui::OpenPopup("Plane properties");
    }
    else if(open_cylinder)
    {
        ImGui::OpenPopup("Cylinder properties");
    }
    else if(open_sphere)
    {
        ImGui::OpenPopup("Sphere properties");
    }

    DrawPlaneProperties();
    DrawCylinderProperties();
    DrawSphereProperties();
}

void PanelResources::DrawPlaneProperties()
{
    if (ImGui::BeginPopupModal("Plane properties", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static float width = 1.0f, height = 1.0f;
        static int slices = 1, stacks = 1;

        ImGui::InputFloat("width", &width);
        ImGui::InputFloat("height", &height);
        ImGui::InputInt("slices", &slices);
        ImGui::InputInt("stacks", &stacks);

        bool close = false;
        close = ImGui::Button("Cancel", ImVec2(128, 0));

        ImGui::SameLine();

        if(ImGui::Button("Ok", ImVec2(128, 0)))
        {
            char lTmp[512];
            sprintf_s(lTmp, 511, "**Plane_%g_%g_%d_%d**", width, height, slices, stacks);

            close = true;
            ResourceMesh::LoadPlane(lTmp, width, height, slices, stacks);
        }

        if(close)
        {
            width = height = 1.0f;
            slices = stacks = 1;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void PanelResources::DrawCylinderProperties()
{
    if (ImGui::BeginPopupModal("Cylinder properties", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static float radius = 0.5f, height = 1.0f;
        static int slices = 20, stacks = 20;

        ImGui::InputFloat("radius", &radius);
        ImGui::InputFloat("height", &height);
        ImGui::InputInt("slices", &slices);
        ImGui::InputInt("stacks", &stacks);

        bool close = false;
        close = ImGui::Button("Cancel", ImVec2(128, 0));

        ImGui::SameLine();

        if(ImGui::Button("Ok", ImVec2(128, 0)))
        {
            char lTmp[512];
            sprintf_s(lTmp, 511, "**Cylinder_%g_%g_%d_%d**", radius, height, slices, stacks);

            close = true;
            ResourceMesh::LoadCylinder(lTmp, height, radius, slices, stacks);
        }

        if(close)
        {
            radius = 0.5f;
            height = 1.0f;
            slices = stacks = 20;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void PanelResources::DrawSphereProperties()
{
    if (ImGui::BeginPopupModal("Sphere properties", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        static float radius = 0.5f;
        static int slices = 20, stacks = 20;

        ImGui::InputFloat("radius", &radius);
        ImGui::InputInt("slices", &slices);
        ImGui::InputInt("stacks", &stacks);

        bool close = false;
        close = ImGui::Button("Cancel", ImVec2(128, 0));

        ImGui::SameLine();

        if(ImGui::Button("Ok", ImVec2(128, 0)))
        {
            char lTmp[512];
            sprintf_s(lTmp, 511, "**Sphere_%g_%d_%d**", radius, slices, stacks);

            close = true;
            ResourceMesh::LoadSphere(lTmp, radius, slices, stacks, 0);
        }

        if(close)
        {
            radius = 0.5f;
            slices = stacks = 20;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

