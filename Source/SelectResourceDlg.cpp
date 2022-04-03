#include "Globals.h"

#include "SelectResourceDlg.h"

#include "Application.h"
#include "ModuleResources.h"

#include "imgui/imgui.h"

#include "Leaks.h"

#include <vector>

void SelectResourceDlg::Open(int resourceType, const char* popupName, int uniqueId)
{
    openUniqueId = uniqueId;
    open_name = std::string(popupName) + std::string("##resources") + std::to_string((size_t)this);
    open_flag = true;
    type = resourceType;
}

void SelectResourceDlg::Display()
{
    if(open_flag)
    {
        ImGui::OpenPopup(open_name.c_str());
        open_flag = false;
    }

    ImGui::SetNextWindowSize(ImVec2(420,300));
    if (ImGui::BeginPopupModal(open_name.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(400, 240), true, ImGuiWindowFlags_NoMove))
        {
            UID r = 0;
            r = DrawResource();

            if(r != 0)
            {
                ImGui::CloseCurrentPopup();
                resource  = r;
                selection = true;
            }
        }
        ImGui::EndChild();

        ImGui::Indent(272);
        if(ImGui::Button("Close", ImVec2(128, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SelectResourceDlg::ClearSelection()
{
    open_name.clear();
    type = 0;
    resource = 0;
    selection = false;
}

UID SelectResourceDlg::DrawResource()
{
    static UID selected = 0;
	std::vector<const Resource*> resources;

	static const char* titles[] = { "Models", "Materials", "Textures", "Meshes", "Audios", "Animation", "State machines", "Others" };

    bool open_tree =ImGui::TreeNodeEx(titles[type], ImGuiTreeNodeFlags_DefaultOpen);

    if (open_tree)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_LIGHT_GREY);
        bool remove = false;
        App->resources->GatherResourceType(resources, (Resource::Type)type);
        for (const Resource* info : resources)
        {
            ImGui::PushID(info->GetExportedFile());

            if (ImGui::TreeNodeEx(info->GetUserResName(), info->GetUID() == selected ? ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_Leaf))
            {
                if (ImGui::IsItemClicked(0) )
                {
                    selected = info->GetUID();
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
                ImGui::TreePop();
            }
            ImGui::PopID();
        }

        ImGui::TreePop();
        ImGui::PopStyleColor();
    }

    return ImGui::IsMouseDoubleClicked(0) ? selected : 0;
}

