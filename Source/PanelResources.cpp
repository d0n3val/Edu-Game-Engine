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

	static bool waiting_to_load_file = false;

	if (waiting_to_load_file == true && App->editor->FileDialog(nullptr, "/Assets/"))
	{
		const char* file = App->editor->CloseFileDialog();
		if (file != nullptr)
        {
            texture_params.file = file;
            App->fs->SplitFilePath(file, nullptr, nullptr, &texture_params.extension);
            Resource::Type type = App->resources->TypeFromExtension(texture_params.extension.c_str());
            if(type == Resource::texture)
            {
                texture_params.compressed = true;
                texture_params.srgb = true;
                texture_params.mipmaps = true;

                ImGui::OpenPopup("Texture properties");
            }
            else
            {
                App->resources->ImportFile(file, false); 
            }
        }
		waiting_to_load_file = false;
	}

	DrawTextureProperties();

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

	DrawResourceType(Resource::mesh, &PanelResources::DrawMeshPopup, false);

    DrawResourceType(Resource::audio);
    DrawResourceType(Resource::animation);

    //ImGui::End();
}


UID PanelResources::DrawResourceType(Resource::Type type, bool opened)
{
    return DrawResourceType(type, nullptr, opened);
}

UID PanelResources::DrawResourceType(Resource::Type type, void (PanelResources::*popup)(void), bool opened)
{
	UID selected = 0;
	vector<const Resource*> resources;

	static const char* titles[] = {
		"Models", "Materials", "Textures", "Meshes", "Audios", "Animation", "Others" };

    bool open_tree =ImGui::TreeNodeEx(titles[type], opened ? ImGuiTreeNodeFlags_DefaultOpen : 0);

    if(popup != nullptr)
    {
        if (ImGui::IsItemClicked(1))
        {
            ImGui::OpenPopup("Resource popup");
        }

        (this->*popup)();
    }

    if (open_tree)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_LIGHT_GREY);
        bool remove = false;
        App->resources->GatherResourceType(resources, type);
        for (vector<const Resource*>::const_iterator it = resources.begin(); !remove && it != resources.end(); ++it)
        {
            const Resource* info = (*it);

            ImGui::PushID(info->GetExportedFile());
            if (ImGui::TreeNodeEx(info->GetName(), ImGuiTreeNodeFlags_Leaf))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_WHITE);
                if (ImGui::IsItemClicked(0))
                {
                    selected = info->GetUID();
                }

                if (ImGui::BeginPopupContextItem())
                {
                    if (true == (remove = ImGui::MenuItem("Remove")))
                    {
                        App->resources->RemoveResource((*it)->GetUID());
                    }
                    ImGui::EndPopup();
                }

                if (ImGui::IsItemHovered())
                {
                    ImGui::BeginTooltip();
                    ImGui::Text("UID: %llu", info->GetUID());
                    ImGui::Text("Source: %s", info->GetFile());
                    ImGui::Text("Exported: %s", info->GetExportedFile());
                    ImGui::Text("References: %u", info->CountReferences());
                    ImGui::EndTooltip();
                }

                ImGui::PopStyleColor();
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::TreePop();
        ImGui::PopStyleColor();
    }

    return selected;
}

void PanelResources::DrawMeshPopup()
{
    bool open_plane = false;
    bool open_cylinder = false;
    bool open_sphere = false;

    if(ImGui::BeginPopup("Resource popup"))
    {
        if (ImGui::BeginMenu("Add prefab"))
        {
            open_plane = ImGui::MenuItem("Plane");
            open_cylinder = ImGui::MenuItem("Cylinder");
            open_sphere = ImGui::MenuItem("Sphere");
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }

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

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel", ImVec2(60, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

}
