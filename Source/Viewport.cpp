#include "Globals.h"
#include "Viewport.h"

#include "Application.h"

#include "ModuleRenderer.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "ModuleDebugDraw.h"
#include "ModuleEditor.h"
#include "GameObject.h"

#include "ComponentCamera.h"

#include "ImGui.h"
#include "GL/glew.h"

Viewport::Viewport()
{
}

Viewport::~Viewport()
{
}

void Viewport::Draw(ComponentCamera* camera)
{
    ImGui::SetNextWindowPos(ImVec2(327.0f, 22.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(884.0f, 574.0f), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Viewport", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing))
	{
        if(ImGui::BeginChild("SceneCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove))
        {
            DrawQuickBar(camera);

            focused = ImGui::IsWindowFocused();

            float width  = ImGui::GetWindowContentRegionWidth();
            float height = ImGui::GetContentRegionAvail().y;

            camera->SetAspectRatio(float(width)/float(height));
            GenerateFBOTexture(unsigned(width), unsigned(height));

            App->renderer->Draw(camera, fbo, fb_width, fb_height);
            App->debug_draw->Draw(camera, fbo, fb_width, fb_height);

            ImGui::GetWindowDrawList()->AddImage(
                    (void*)fb_tex,
                    ImVec2(ImGui::GetCursorScreenPos()),
                    ImVec2(ImGui::GetCursorScreenPos().x + fb_width,
                        ImGui::GetCursorScreenPos().y + fb_height), 
                    ImVec2(0, 1), ImVec2(1, 0));

            DrawGuizmo(camera);
        }
        ImGui::EndChild();
    }
	ImGui::End();
}

void Viewport::GenerateFBOTexture(unsigned w, unsigned h)
{
    if(w != fb_width || h != fb_height)
    {
        if(fb_tex != 0)
        {
            glDeleteTextures(1, &fb_tex);
        }

        if(w != 0 && h != 0)
        {
            if(fbo == 0)
            {
                glGenFramebuffers(1, &fbo);
            }

            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glGenTextures(1, &fb_tex);
            glBindTexture(GL_TEXTURE_2D, fb_tex);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            glGenRenderbuffers(1, &fb_depth);
            glBindRenderbuffer(GL_RENDERBUFFER, fb_depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb_depth);            
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb_tex, 0);

            glDrawBuffer(GL_COLOR_ATTACHMENT0);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

		fb_width = w;
		fb_height = h;
    }
}

void Viewport::DrawQuickBar(ComponentCamera* camera)
{
    Application::State state = App->GetState();

    if (state != Application::play && state != Application::pause)
    {
        if (ImGui::Button("PLAY", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
            App->Play();
    }
    else
    {
        if (ImGui::Button("STOP", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
            App->Stop();
    }

    ImGui::SameLine();

    if (state == Application::play)
    {
        if (ImGui::Button("PAUSE", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
            App->Pause();
    }
    else if(state == Application::pause)
    {
        if (ImGui::Button("CONTINUE", ImVec2(60, 22)) || App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN)
            App->UnPause();
    }

    ImGui::SameLine(150.f);
    ImGui::Checkbox("Grid", &App->renderer3D->draw_plane);
    ImGui::SameLine();
    ImGui::Checkbox("Axis", &App->renderer3D->draw_axis);
    ImGui::SameLine();
    ImGui::Checkbox("Dbg Draw", &App->renderer3D->debug_draw);

	ImGui::SameLine();
    ImGui::ColorEdit3("Background", (float*)&camera->background, ImGuiColorEditFlags_NoInputs);

    ImGui::SameLine(0, 50);
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

}

void Viewport::DrawGuizmoProperties(GameObject* go)
{
    bool local = guizmo_mode == ImGuizmo::LOCAL && guizmo_op != ImGuizmo::SCALE;

    float4x4 model = local ? go->GetLocalTransform() : go->GetGlobalTransformation();
    model.Transpose();

    ImGui::RadioButton("Translate", (int*)&guizmo_op, (int)ImGuizmo::TRANSLATE);
    ImGui::SameLine();
    ImGui::RadioButton("Rotate", (int*)&guizmo_op, ImGuizmo::ROTATE);

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents((float*)&model, matrixTranslation, matrixRotation, matrixScale);
    bool transform_changed = ImGui::DragFloat3("Tr", matrixTranslation, 3);
    transform_changed = transform_changed || ImGui::DragFloat3("Rt", matrixRotation, 3);

    if(transform_changed)
    {
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&model);
        model.Transpose();

        if(local || go->GetParent() == nullptr)
        {
            go->SetLocalTransform(model);
        }
        else
        {
            float4x4 parent = go->GetParent()->GetGlobalTransformation();
			parent.InverseOrthonormal();
            go->SetLocalTransform(parent*model);
        }
    }

    if (guizmo_op != ImGuizmo::SCALE)
    {
        ImGui::RadioButton("Local", (int*)&guizmo_mode, (int)ImGuizmo::LOCAL);
        ImGui::SameLine();
        ImGui::RadioButton("World", (int*)&guizmo_mode, (int)ImGuizmo::WORLD);
    }

    ImGui::PushID("snap");
    ImGui::Checkbox("", &guizmo_useSnap);
    ImGui::PopID();
    ImGui::SameLine();

    switch (guizmo_op)
    {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &guizmo_snap.x);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &guizmo_snap.x);
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat("Scale Snap", &guizmo_snap.x);
            break;
    }

}

void Viewport::DrawGuizmo(ComponentCamera* camera)
{
	if (App->editor->selection_type == ModuleEditor::SelectionGameObject && App->editor->selected.go != nullptr)
	{
		float4x4 view = camera->GetOpenGLViewMatrix();
		float4x4 proj = camera->GetOpenGLProjectionMatrix();

		ImGuizmo::BeginFrame();
		ImGuizmo::Enable(true);

        float4x4 model = App->editor->selected.go->GetGlobalTransformation();
        model.Transpose();

		float4x4 delta;

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(float(ImGui::GetCursorScreenPos().x), float(ImGui::GetCursorScreenPos().y), float(fb_width), float(fb_height));
		ImGuizmo::SetDrawlist();
		ImGuizmo::Manipulate((const float*)&view, (const float*)&proj, guizmo_op, guizmo_mode, (float*)&model, (float*)&delta, guizmo_useSnap ? &guizmo_snap.x : NULL);

		if (ImGuizmo::IsUsing() && !delta.IsIdentity())
		{
			model.Transpose();
            if(App->editor->selected.go->GetParent() == nullptr)
            {
                App->editor->selected.go->SetLocalTransform(model);
            }
            else
            {
                float4x4 parent = App->editor->selected.go->GetParent()->GetGlobalTransformation();
                parent.InverseOrthonormal();
                App->editor->selected.go->SetLocalTransform(parent*model);
            }
        }
	}
}
