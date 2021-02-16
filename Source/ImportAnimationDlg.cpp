#include "Globals.h"

#include "ImportAnimationDlg.h"

#include "imgui/imgui.h"

#include <algorithm>

void ImportAnimationDlg::Open(const std::string& _file, const std::string& name)
{
    ImGui::OpenPopup("Animation properties");
    file = _file;
    clips_name = name;

    clips.resize(1);
    strcpy_s(clips[0].name, clips_name.c_str());
}

void ImportAnimationDlg::Display()
{
    ImGui::SetNextWindowSize(ImVec2(400, 450));
    if (ImGui::BeginPopupModal("Animation properties", nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(380, 390), true, ImGuiWindowFlags_NoMove))
        {
            ImGui::LabelText("File", file.c_str());

            ImGui::InputFloat("Scale", &scale);

            int num_clips = clips.size();

            if(ImGui::InputInt("# Clips", &num_clips))
            {
                uint new_size  = std::max(1, num_clips);
                uint prev_size = clips.size();

                clips.resize(new_size);

                for(uint i=prev_size; i< new_size; ++i)
                {
                    strcpy_s(clips[i].name, clips_name.c_str());
                }
            }

            for(uint i=0; i< clips.size(); ++i)
            {
                ImGui::PushID(i);
                if(ImGui::BeginChild("Clip", ImVec2(350, 100), true, ImGuiWindowFlags_NoMove))
                {
                    ImGui::InputText("Clip name", clips[i].name, 128);
                    ImGui::InputInt("First frame", (int*)&clips[i].first);
                    ImGui::InputInt("Last frame", (int*)&clips[i].last);
                }
                ImGui::EndChild();
                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        ImGui::Indent(252);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            selection = true;
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

void ImportAnimationDlg::ClearSelection()
{
    file.clear();
    clips.clear();
    clips_name.clear();
    selection = false;
}
