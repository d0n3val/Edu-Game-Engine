#include "Globals.h"

#include "ShowTextureDlg.h"

#include "ImGui/imgui.h"

#include "OGL.h"

ShowTextureDlg::ShowTextureDlg()
{
    open_name = std::string("Show texture") + std::string("##textures") + std::to_string((size_t)this);
}

void ShowTextureDlg::Open(Texture2D* _texture, uint _width, uint _height)
{
    open_flag = true;
    texture   = _texture;
    width     = _width;
    height    = _height;
}

void ShowTextureDlg::Display()
{
    if(open_flag)
    {
        ImGui::OpenPopup(open_name.c_str());
        open_flag = false;
    }

    ImGui::SetNextWindowSize(ImVec2(200, 150));
    if (ImGui::BeginPopupModal(open_name.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(180, 90), true, ImGuiWindowFlags_NoMove))
        {
            ImVec2 size = ImVec2(float(width), float(height));

            ImGui::Image((ImTextureID)texture->Id(), size, ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::EndChild();

        ImGui::PushItemWidth(96);
        if(ImGui::DragFloat("Zoom", &zoom))
        {
			//preview_zoom = std::max(preview_zoom, 0.0f);
            //GeneratePreview(texture->GetWidth(), texture->GetHeight(), static_cast<Texture2D*>(texture->GetTexture()), mesh);
        }
        ImGui::PushItemWidth(0);

        ImGui::SameLine();

        if(ImGui::Checkbox("show texture", &show_texture) )
        {
            //GeneratePreview(texture->GetWidth(), texture->GetHeight(), static_cast<Texture2D*>(texture->GetTexture()), mesh);
        }

        ImGui::SameLine();

        if(ImGui::Checkbox("show uvs", &show_uvs) )
        {
            //GeneratePreview(texture->GetWidth(), texture->GetHeight(), static_cast<Texture2D*>(texture->GetTexture()), mesh);
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(96);

        if(ImGui::Combo("Set",(int*)&uv_set, "UV set 0\0UV set 1"))
        {
            //GeneratePreview(texture->GetWidth(), texture->GetHeight(), static_cast<Texture2D*>(texture->GetTexture()), mesh);
        }

        ImGui::PushItemWidth(0.0);

        ImGui::SameLine();

        if(ImGui::ColorEdit4("uv color", (float*)&uv_color, ImGuiColorEditFlags_NoInputs) && show_uvs)
        {
            //GeneratePreview(texture->GetWidth(), texture->GetHeight(), static_cast<Texture2D*>(texture->GetTexture()), mesh);
        }

        ImGui::SameLine();

        //ImGui::Indent(tex_size.x-128);
        if(ImGui::Button("Close", ImVec2(128, 0)))
        {
            ImGui::CloseCurrentPopup();
        }


        ImGui::EndPopup();
    }
}

void ShowTextureDlg::Clear()
{
    texture   = nullptr;
    width     = 0;
    height    = 0;
}
