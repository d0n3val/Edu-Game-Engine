#include "Globals.h"

#include "ImportModelDlg.h"

#include "imgui/imgui.h"

#include "Application.h"
#include "ModuleFileSystem.h"

ImportModelDlg::ImportModelDlg()
{
    open_name = std::string("Model properties") + std::string("##models") + std::to_string((size_t)this);
}

void ImportModelDlg::Open(const std::string& _file)
{
    open_flag = true;
    file      = _file;

    App->fs->SplitFilePath(file.c_str(), nullptr, &user_name, nullptr);

    size_t pos_dot = user_name.find_last_of(".");
    if(pos_dot != std::string::npos)
    {
        user_name.erase(user_name.begin()+pos_dot, user_name.end());
    }
}

void ImportModelDlg::Display()
{
    if(open_flag)
    {
        ImGui::OpenPopup(open_name.c_str());
        open_flag = false;
    }

    ImGui::SetNextWindowSize(ImVec2(300, 150));

    if (ImGui::BeginPopupModal(open_name.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(280, 90), true, ImGuiWindowFlags_NoMove))
        {
            ImGui::InputFloat("Scale", &scale);

            char lTmp[256];
            strcpy_s(&lTmp[0], 255, user_name.c_str());
            if(ImGui::InputText("User Name", lTmp, 256))
            {
                user_name = &lTmp[0];
            }
        }
        ImGui::EndChild();

        ImGui::Indent(150);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            selection = true;

            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(60, 0)))
        {
            ClearSelection();

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ImportModelDlg::ClearSelection()
{
    file.clear();

    scale     = 1.0f;
    selection = false;
}
