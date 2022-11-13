#include "Globals.h"

#include "ImportTextureDlg.h"

#include "imgui/imgui.h"

ImportTexturesDlg::ImportTexturesDlg()
{
    open_name = std::string("Texture properties") + std::string("##textures") + std::to_string((size_t)this);
}

void ImportTexturesDlg::Open(const std::string& _file)
{
    open_flag = true;
    file = _file;
}

void ImportTexturesDlg::Display()
{
    if(open_flag)
    {
        ImGui::OpenPopup(open_name.c_str());
        open_flag = false;
    }

    ImGui::SetNextWindowSize(ImVec2(300, 250));
    if (ImGui::BeginPopupModal(open_name.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(280, 190), true, ImGuiWindowFlags_NoMove))
        {
            ImGui::Checkbox("Mipmaps", &mipmaps);
            ImGui::Checkbox("Equirectangular to Cubemap", &toCubemap);
        }
        ImGui::EndChild();

        ImGui::Indent(220);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            selection = true;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void ImportTexturesDlg::ClearSelection()
{
    file.clear();
    mipmaps    = true;
    toCubemap  = false;
    selection  = false;
}
