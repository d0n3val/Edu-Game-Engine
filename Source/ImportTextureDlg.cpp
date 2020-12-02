#include "Globals.h"

#include "ImportTextureDlg.h"

#include "imgui/imgui.h"

void ImportTexturesDlg::Open(const std::string& _file)
{
    ImGui::OpenPopup("Texture properties");
    file = _file;
}

void ImportTexturesDlg::Display()
{
    ImGui::SetNextWindowSize(ImVec2(200, 150));
    if (ImGui::BeginPopupModal("Texture properties", nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(180, 90), true, ImGuiWindowFlags_NoMove))
        {
            ImGui::Checkbox("Compressed", &compressed);
            ImGui::Checkbox("Mipmaps", &mipmaps);
            ImGui::Checkbox("sRGB", &srgb);
        }
        ImGui::EndChild();

        ImGui::Indent(52);
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

void ImportTexturesDlg::ClearSelection()
{
    file.clear();
    compressed = true;
    mipmaps    = true;
    srgb       = true;
    selection  = false;
}
