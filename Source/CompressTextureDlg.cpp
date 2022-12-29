#include "Globals.h"

#include "CompressTextureDlg.h"
#include "imgui/imgui.h"
#include "ResourceTexture.h"

#include "Leaks.h"

namespace
{
    const char* names[] = { "Colour", "Grayscale", "Normals", "HDR", "Colour High Quality", "Colour High Quality (faster)"};
}

CompressTextureDlg::CompressTextureDlg()
{
    open_name = std::string("Compression properties") + std::string("##compress") + std::to_string((size_t)this);
}

void CompressTextureDlg::Open(int uniqueId)
{
    openUniqueId = uniqueId;
    open_flag = true;
}

void CompressTextureDlg::Display()
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
            assert(sizeof(names) / sizeof(const char*) == ResourceTexture::Compress_Count);
            ImGui::Combo("Format", &type, &names[0], ResourceTexture::Compress_Count, -1);
        }

        ImGui::EndChild();

        ImGui::Indent(150);
        if(ImGui::Button("Ok", ImVec2(60, 0)))
        {
            selection = true;

            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel", ImVec2(60, 0)))
        {
            selection = false;

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void CompressTextureDlg::ClearSelection()
{
    selection  = false;
}
