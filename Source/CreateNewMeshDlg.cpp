#include "Globals.h"

#include "CreateNewMeshDlg.h"
#include "ResourceMesh.h"

#include <imgui.h>

CreateNewMeshDlg::CreateNewMeshDlg()
{
    openName = std::string("Create New Mesh properties") + std::string("##meshes") + std::to_string((size_t)this);
}

void CreateNewMeshDlg::Open()
{
    openFlag = true;
}

void CreateNewMeshDlg::Display()
{
    if(openFlag)
    {
        ImGui::OpenPopup(openName.c_str());
        openFlag = false;
    }

    ImGui::SetNextWindowSize(ImVec2(300, 250));
    if (ImGui::BeginPopupModal(openName.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(280, 190), true, ImGuiWindowFlags_NoMove))
        {
            ImGui::Combo("Type", (int*)&typeMesh, "Spot Cone");
            switch(typeMesh)
            {
                case MeshType_SpotCone:
                {
                    ImGui::InputText("Name", &params.name[0], 1024);
                    ImGui::InputInt("Num slices", (int*)&params.slices);
                    ImGui::InputInt("Num stacks", (int*)&params.stacks);
                    ImGui::InputFloat("Height", &params.height);
                    ImGui::InputFloat("Radius", &params.radius);

                    break;
                }
            }
        }
        ImGui::EndChild();

        ImGui::Indent(220);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            meshID = ResourceMesh::LoadCone(params.name, params.height, params.radius, params.slices, params.stacks);

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void CreateNewMeshDlg::Clear()
{
    meshID = 0;
}
