#include "Globals.h"

#include "ShowTextureDlg.h"
#include "ResourceMesh.h"
#include "ResourceTexture.h"

#include "Application.h"
#include "ModulePrograms.h"
#include "ModuleInput.h"
#include "OGL.h"
#include "OpenGL.h"

#include "ImGui/imgui.h"

#include "Leaks.h"

#include <algorithm>

#define CANVAS_SIZE 685

ShowTextureDlg::ShowTextureDlg()
{
    open_name = std::string("Show texture") + std::string("##textures") + std::to_string((size_t)this);
}

void ShowTextureDlg::Open(const ResourceMesh* _mesh, ResourceTexture* _texture)
{
    open_flag = true;
    mesh      = _mesh;
    source    = _texture;

    if(source)
    {
        source->LoadToMemory();
    }

    width     = source->GetWidth();
    height    = source->GetHeight();

    zoom = float(uint(min(CANVAS_SIZE / float(width), CANVAS_SIZE / float(height))*100.0f));
}

void ShowTextureDlg::Display()
{
    if(open_flag)
    {
        ImGui::OpenPopup(open_name.c_str());
        open_flag = false;

        if(source->GetType() == ResourceTexture::Texture2D)
        {
            GeneratePreview();
        }
    }

    bool generate = false;

    ImGui::SetNextWindowSize(ImVec2(702, 748));
    if (ImGui::BeginPopupModal(open_name.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(CANVAS_SIZE, CANVAS_SIZE), true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_HorizontalScrollbar))
        {
            ImVec2 size = ImVec2(width * zoom * 0.01f, height * zoom * 0.01f);

            ImGui::Image((ImTextureID)target->Id(), size, ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::EndChild();

        ImGui::PushItemWidth(128);
        generate = ImGui::DragFloat("Zoom", &zoom) || generate;
        zoom = max(zoom, 0.0f);
        ImGui::PushItemWidth(0);

        ImGui::SameLine();

        generate = ImGui::Checkbox("show texture", &show_texture) || generate;

        ImGui::SameLine();

        if(mesh)
        {
            generate = ImGui::Checkbox("show uvs", &show_uvs) ||  generate;

            ImGui::SameLine();
            ImGui::PushItemWidth(96);

            generate = ImGui::Combo("Set",(int*)&uv_set, "UV set 0\0UV set 1") || generate;

            ImGui::PushItemWidth(0.0);

            ImGui::SameLine();

            generate = (ImGui::ColorEdit4("uv color", (float*)&uv_color, ImGuiColorEditFlags_NoInputs) && show_uvs) || generate;
        }

        ImGui::SameLine();

        if(ImGui::Button("Close", ImVec2(98, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        if (App->input->GetMouseWheel() != 0)
        {
            generate = true;
        }

        ImGui::EndPopup();
    }

    generate = generate && source->GetType() == ResourceTexture::Texture2D;

    if(generate)
    {
        GeneratePreview();
    }
}

void ShowTextureDlg::Clear()
{
    mesh    = nullptr;

    if(source)
    {
        source->Release();
    }

    source  = nullptr;
    width   = 0;
    height  = 0;
}

void ShowTextureDlg::GeneratePreview()
{
    GenerateTargetFB();

    if(show_texture)
    {
        GenerateSourceFB();
        source_fb->BlitTo(target_fb.get(), 0, 0, width, height, 0, 0, uint(width * zoom * 0.01f), uint(height * zoom * 0.01f), GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

    if(show_uvs)
    {
        DrawPreviewUVs();
    }
}

void ShowTextureDlg::GenerateSourceFB()
{
    source_fb = std::make_unique<Framebuffer>();
    assert(source->GetType() == ResourceTexture::Texture2D);
    source_fb->AttachColor(static_cast<Texture2D*>(source->GetTexture()), 0, 0);
}

void ShowTextureDlg::GenerateTargetFB()
{
    // \todo: compute zoom width and height

    target_fb   = std::make_unique<Framebuffer>(); 
    target      = std::make_unique<Texture2D>(GL_TEXTURE_2D, uint(width*zoom*0.01f), uint(height*zoom*0.01f), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, false);

    target_fb->AttachColor(target.get());
}

void ShowTextureDlg::DrawPreviewUVs()
{
    target_fb->Bind();
    glViewport(0, 0, uint(width * zoom * 0.01f), uint(height * zoom * 0.01f));
    glClear(GL_DEPTH_BUFFER_BIT);
    if(!show_texture)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    App->programs->UseProgram("show_uvs", uv_set);
    glUniform4fv(0, 1, (const float*)&uv_color);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	mesh->Draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
	App->programs->UnuseProgram();
}

