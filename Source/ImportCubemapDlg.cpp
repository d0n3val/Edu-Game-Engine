#include "Globals.h"

#include "ImportCubemapDlg.h"

#include "imgui/imgui.h"

namespace
{
    const char* side_names[ImportCubemapDlg::SideCount] = { "Front :", "Back  :", "Left  :", "Right :", "Top   :", "Bottom:" };
}

void ImportCubemapDlg::Open()
{
    fileDialog.SetPwd(std::filesystem::path("Assets"));
    open_flag = true;
}

void ImportCubemapDlg::Display()
{
    fileDialog.Display();
    if (fileDialog.HasSelected())
    {
        files[open_index] = fileDialog.GetSelected().generic_string();
        fileDialog.ClearSelected();
        open_flag = true;
    }

    if (in_file_flag && !fileDialog.IsOpened())
    {
        open_flag = true;
        in_file_flag = false;
    }

    if(open_flag)
    {
        ImGui::OpenPopup("Cubemap");
        open_flag = false;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 210));
    if (ImGui::BeginPopupModal("Cubemap", nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(780, 150), true, ImGuiWindowFlags_NoMove))
        {
            for(uint i=0; i<SideCount; ++i)
            {
                ImGui::PushID(i);
                if(ImGui::Button("Change"))
                {
                    open_index = i;
                    fileDialog.Open();
                    in_file_flag = true;
                }
                ImGui::PopID();
                ImGui::SameLine();
                ImGui::TextColored(IMGUI_YELLOW, side_names[i]);
                ImGui::SameLine();
                ImGui::Text("%s", files[i].c_str());
            }
        }
        ImGui::EndChild();

        ImGui::Indent(652);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            bool ok = true;
            for(uint i=0; ok && i<SideCount; ++i)
            {
                ok = !files[i].empty();
            }
            
            if(ok)
            {
                selection = true;
            }
            else
            {
                ClearSelection();
            }

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel", ImVec2(60, 0)))
        {
            ClearSelection();

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ImportCubemapDlg::ClearSelection()
{
    for(uint i=0; i< SideCount; ++i)
    {
        files[i].clear();
    }

    selection = false;
}

