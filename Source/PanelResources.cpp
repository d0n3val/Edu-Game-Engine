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
    //ImGui::Begin("Resources", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing );

	if (waiting_to_load != Resource::unknown && App->editor->FileDialog(nullptr, "/Assets/"))
	{
		const char* file = App->editor->CloseFileDialog();
		if (file != nullptr)
        {
            switch(waiting_to_load)
            {
                case Resource::texture:
                {
                    texture_params.file = file;

                    ImGui::OpenPopup("Texture properties");
                    break;
                }
                case Resource::animation:
                {
                    animation_params.file = file;
                    std::string user_name;
                    App->fs->SplitFilePath(file, nullptr, &user_name, nullptr);

                    size_t pos_dot = user_name.find_last_of(".");
                    if(pos_dot != std::string::npos)
                    {
                        user_name.erase(user_name.begin()+pos_dot, user_name.end());
                    }

                    strcpy_s(animation_params.clips[0].name, user_name.c_str());
                    strcpy_s(animation_params.clip_names, user_name.c_str());

                    ImGui::OpenPopup("Animation properties");
                    break;
                }
                default:
                    App->resources->ImportFile(file, waiting_to_load, false); 
                    break;
            }
        }
        waiting_to_load = Resource::unknown;
	}

	DrawTextureProperties();
	DrawAnimationProperties();

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

	DrawResourceType(Resource::model, &PanelResources::DrawResourcePopup);
	DrawResourceType(Resource::material, &PanelResources::DrawResourcePopup);
	DrawResourceType(Resource::texture, &PanelResources::DrawResourcePopup);

	DrawResourceType(Resource::mesh, &PanelResources::DrawMeshPopup);

    DrawResourceType(Resource::audio, &PanelResources::DrawResourcePopup);
    DrawResourceType(Resource::animation, &PanelResources::DrawResourcePopup);
    DrawResourceType(Resource::state_machine, &PanelResources::DrawResourcePopup);

    //ImGui::End();
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

            ImGui::PushID(info->GetExportedFile());

			bool selected = selection.find(info->GetUID()) != selection.end();

            if (ImGui::TreeNodeEx(info->GetName(), selected ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_Leaf))
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
            ResourceMesh::LoadSphere(lTmp, radius, slices, stacks);
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

void PanelResources::DrawTextureProperties()
{
    ImGui::SetNextWindowSize(ImVec2(200, 150));
    if (ImGui::BeginPopupModal("Texture properties", nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(180, 90), true, ImGuiWindowFlags_NoMove))
        {
            ImGui::Checkbox("Compressed", &texture_params.compressed);
            ImGui::Checkbox("Mipmaps", &texture_params.mipmaps);
            ImGui::Checkbox("sRGB", &texture_params.srgb);
        }
        ImGui::EndChild();

        ImGui::Indent(52);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            App->resources->ImportTexture(texture_params.file.c_str(), texture_params.compressed, texture_params.mipmaps, texture_params.srgb); 
            texture_params.Reset();

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel", ImVec2(60, 0)))
        {
            texture_params.Reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void PanelResources::DrawAnimationProperties()
{
    ImGui::SetNextWindowSize(ImVec2(400, 450));
    if (ImGui::BeginPopupModal("Animation properties", nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(380, 390), true, ImGuiWindowFlags_NoMove))
        {
            ImGui::LabelText("File", animation_params.file.c_str());
            int num_clips = animation_params.clips.size();
            if(ImGui::InputInt("# Clips", &num_clips))
            {
                uint new_size = max(1, num_clips);
                uint prev_size = animation_params.clips.size();
                animation_params.clips.resize(new_size);

                for(uint i=prev_size; i< new_size; ++i)
                {
                    strcpy_s(animation_params.clips[i].name, animation_params.clip_names);
                }
            }

            for(uint i=0; i< animation_params.clips.size(); ++i)
            {
                ImGui::PushID(i);
                if(ImGui::BeginChild("Clip", ImVec2(350, 100), true, ImGuiWindowFlags_NoMove))
                {
                    ImGui::InputText("Clip name", animation_params.clips[i].name, 128);
                    ImGui::InputInt("First frame", (int*)&animation_params.clips[i].first);
                    ImGui::InputInt("Last frame", (int*)&animation_params.clips[i].last);
                }
                ImGui::EndChild();
                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        ImGui::Indent(252);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            for(uint i=0; i< animation_params.clips.size(); ++i)
            {
                App->resources->ImportAnimation(animation_params.file.c_str(), animation_params.clips[i].first, 
                        animation_params.clips[i].last, animation_params.clips[i].name); 
            }
            animation_params.Reset();

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel", ImVec2(60, 0)))
        {
            animation_params.Reset();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

