#include "Globals.h"
#include "Viewport.h"

#include "Application.h"

#include "ModuleRenderer.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "ModuleDebugDraw.h"
#include "ModuleEditor.h"
#include "ModuleHints.h"

#include "GameObject.h"
#include "PointLight.h"

#include "ComponentCamera.h"

#include "Config.h"
#include "DebugDraw.h"

#include "ImGui.h"
#include "GL/glew.h"

Viewport::Viewport()
{
}

Viewport::~Viewport()
{
    if(msfb_color != 0)
    {
        glDeleteRenderbuffers(1, &msfb_color);
    }

    if(msfbo != 0)
    {
        glDeleteFramebuffers(1, &msfbo);
    }

    if(msfb_depth != 0)
    {
        glDeleteRenderbuffers(1, &msfb_depth);
    }

    if(fb_tex != 0)
    {
        glDeleteTextures(1, &fb_tex);
    }

    if(fbo != 0)
    {
        glDeleteFramebuffers(1, &fbo);
    }

    if(fb_depth != 0)
    {
        glDeleteRenderbuffers(1, &fb_depth);
    }
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

            float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);
            if (draw_plane == true)
            {
                dd::xzSquareGrid(-50.0f*metric_proportion, 50.0f*metric_proportion, 0.0f, metric_proportion, dd::colors::LightGray, 0, true);
            }

            if(draw_axis == true)
            {
                dd::axisTriad(math::float4x4::identity, metric_proportion*0.1f, 100.0f, 0, false);
            }

            if (debug_draw == true)
            {
                App->DebugDraw();
            }

            App->renderer->Draw(camera, msaa ? msfbo : fbo, fb_width, fb_height);
            App->debug_draw->Draw(camera, msaa ? msfbo : fbo, fb_width, fb_height);

            ImVec2 screenPos = ImGui::GetCursorScreenPos();

            if(msaa)
            {
                glBindFramebuffer(GL_READ_FRAMEBUFFER, msfbo);
                glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
                glBlitFramebuffer(0, 0, fb_width, fb_height, 0, 0, fb_width, fb_height, GL_COLOR_BUFFER_BIT, GL_NEAREST); 
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            ImGui::GetWindowDrawList()->AddImage(
                    (void*)fb_tex,
                    ImVec2(screenPos),
                    ImVec2(screenPos.x + fb_width, screenPos.y + fb_height), 
                    ImVec2(0, 1), ImVec2(1, 0));

            DrawGuizmo(camera);

        }
        ImGui::EndChild();
    }
	ImGui::End();
}

void Viewport::Save(Config* config) const
{
    config->AddBool("Draw Plane", draw_plane);
    config->AddBool("Draw Axis", draw_axis);
    config->AddBool("Debug Draw", debug_draw);
    config->AddBool("MSAA", msaa);
}

void Viewport::Load(Config* config)
{
	draw_plane = config->GetBool("Draw Plane", true);
	draw_axis = config->GetBool("Draw Axis", true);
	debug_draw = config->GetBool("Debug Draw", true);
    msaa = config->GetBool("MSAA", true);
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

            GenerateFBOMultisampled(w, h);
        }

		fb_width = w;
		fb_height = h;
    }
}

void Viewport::GenerateFBOMultisampled(unsigned w, unsigned h)
{
    if(msfb_color != 0)
    {
        glDeleteRenderbuffers(1, &msfb_color);
    }

    assert(w != 0 && h != 0);

    if(msfbo == 0)
    {
        glGenFramebuffers(1, &msfbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, msfbo);

    glGenRenderbuffers(1, &msfb_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, msfb_depth);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msfb_depth);            
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenRenderbuffers(1, &msfb_color);
    glBindRenderbuffer(GL_RENDERBUFFER, msfb_color);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msfb_color);            
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

void Viewport::DrawQuickBar(ComponentCamera* camera)
{
    Application::State state = App->GetState();

    if(ImGui::BeginChild("ToolCanvas", ImVec2(435, 38), true, ImGuiWindowFlags_NoMove))
    {
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

        ImGui::SameLine(75.f);
        ImGui::Checkbox("Grid", &draw_plane);
        ImGui::SameLine();
        ImGui::Checkbox("Axis", &draw_axis);
        ImGui::SameLine();
        ImGui::Checkbox("Dbg Draw", &debug_draw);
        ImGui::SameLine();
        ImGui::Checkbox("MSAA", &msaa);

        ImGui::SameLine();
        ImGui::ColorEdit3("Background", (float*)&camera->background, ImGuiColorEditFlags_NoInputs);
    }
    ImGui::EndChild();

	ImGui::SameLine();
    if(ImGui::BeginChild("ScaleCanvas", ImVec2(145, 38), true, ImGuiWindowFlags_NoMove))
    {
        float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);
        if(ImGui::DragFloat("Scale", &metric_proportion))
        {
            App->hints->SetFloatValue(ModuleHints::METRIC_PROPORTION, metric_proportion);
        }
    }
    ImGui::EndChild();

	ImGui::SameLine();
    if(ImGui::BeginChild("TextCanvas", ImVec2(350, 38), true, ImGuiWindowFlags_NoMove))
    {
        ImGui::SameLine();
        ImGui::Text("Avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }
    ImGui::EndChild();

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

void Viewport::DrawGuizmoProperties(PointLight* point)
{
    bool local = guizmo_mode == ImGuizmo::LOCAL && guizmo_op != ImGuizmo::SCALE;

    guizmo_mode = ImGuizmo::WORLD;
    guizmo_op = ImGuizmo::TRANSLATE;

    float3 position = point->GetPosition();

    if(ImGui::DragFloat3("Position", (float*)&position, 3))
    {
        point->SetPosition(position);
    }

    ImGui::PushID("snap");
    ImGui::Checkbox("", &guizmo_useSnap);
    ImGui::PopID();
    ImGui::SameLine();
    ImGui::InputFloat3("Snap", &guizmo_snap.x);
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
	else if (App->editor->selection_type == ModuleEditor::SelectionPointLight && App->editor->selected.point != nullptr)
    {
		float4x4 view = camera->GetOpenGLViewMatrix();
		float4x4 proj = camera->GetOpenGLProjectionMatrix();

		ImGuizmo::BeginFrame();
		ImGuizmo::Enable(true);

        float4x4 model = float4x4::identity;
        model.SetTranslatePart(App->editor->selected.point->GetPosition());
        model.Transpose();

		float4x4 delta;

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(float(ImGui::GetCursorScreenPos().x), float(ImGui::GetCursorScreenPos().y), float(fb_width), float(fb_height));
		ImGuizmo::SetDrawlist();
		ImGuizmo::Manipulate((const float*)&view, (const float*)&proj, guizmo_op, guizmo_mode, (float*)&model, (float*)&delta, guizmo_useSnap ? &guizmo_snap.x : NULL);

		if (ImGuizmo::IsUsing() && !delta.IsIdentity())
		{
			model.Transpose();
            App->editor->selected.point->SetPosition(model.TranslatePart());
        }
    }
}
