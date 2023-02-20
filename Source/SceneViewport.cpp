#include "Globals.h"
#include "SceneViewport.h"

#include "Application.h"

#include "ModuleRenderer.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"
#include "ModuleDebugDraw.h"
#include "ModuleEditor.h"
#include "ModuleEditorCamera.h"
#include "ModuleHints.h"
#include "ModulePrograms.h"
#include "ModuleLevelManager.h"
#include "Postprocess.h"
#include "DepthPrepass.h"
#include "GBufferExportPass.h"
#include "ShadowmapPass.h"
#include "CascadeShadowPass.h"
#include "ScreenSpaceAO.h"
#include "FxaaPass.h"

#include "GameObject.h"

#include "PointLight.h"
#include "SpotLight.h"

#include "ComponentCamera.h"
#include "ComponentMeshRenderer.h"
#include "ResourceMesh.h"

#include "Config.h"
#include "DebugDraw.h"

#include "ImGui.h"
#include "GL/glew.h"

#include <variant>

#include "Leaks.h"

SceneViewport::SceneViewport()
{        
    // first row ==> positions, second row ==> uv�s
    static const float vertices[] = { -40.0f,  0.0f, 40.0f , 40.0f,  0.0f, 40.0f , 40.0f, 0.0f, -40.0f , -40.0f, 0.0f, -40.0f };

    static const unsigned indices[] = { 0, 1, 2, 0, 2, 3 };

    static VertexAttrib attribs[] = { {0, 3, GL_FLOAT, false, sizeof(float) * 3, 0 } };

    grid_vbo.reset(Buffer::CreateVBO(GL_STATIC_DRAW, sizeof(vertices), (void*)vertices));
    grid_ibo.reset(Buffer::CreateIBO(GL_STATIC_DRAW, sizeof(indices), (void*)indices));
    grid_vao = std::make_unique<VertexArray>(grid_vbo.get(), grid_ibo.get(), attribs, 1);

}

SceneViewport::~SceneViewport()
{
}

void SceneViewport::Draw(ComponentCamera* camera, ComponentCamera* culling)
{
    if(ImGui::BeginChild("SceneCanvas", ImVec2(0, 0), true, ImGuiWindowFlags_NoMove))
    {
		focused = ImGui::IsWindowFocused();

        DrawQuickBar(camera);

        float width  = ImGui::GetWindowContentRegionWidth();
        float height = ImGui::GetContentRegionAvail().y;

        camera->SetAspectRatio(float(width)/float(height));
        GenerateFBOs(unsigned(width), unsigned(height));

        float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);

        if(draw_axis == true)
        {
            dd::axisTriad(math::float4x4::identity, metric_proportion*0.1f, metric_proportion, 0, false);
        }
        if (draw_plane == true)
        {
            dd::xzSquareGrid(-100.0f, 100.0f, 0.0f, 1.0f, dd::colors::Gray);
        }

        if (debug_draw == true)
        {
            App->DebugDraw();
        }

        ImVec2 cursor = ImGui::GetCursorScreenPos();
        ImVec2 mouse = ImGui::GetMousePos();
        ImVec2 rel_position = ImVec2(mouse.x-cursor.x, mouse.y-cursor.y);

        if(focused && ImGui::IsMouseClicked(0, false) && rel_position.x >= 0 && rel_position.x <= width && rel_position.y >= 0 && rel_position.y <= height)
        {
            PickSelection(camera, (int)rel_position.x, (int)rel_position.y);
        }

        bool msaa = App->hints->GetBoolValue(ModuleHints::ENABLE_MSAA);

        Framebuffer* framebuffer = msaa ? framebuffers[FRAMEBUFFER_MSAA].framebuffer.get() : framebuffers[FRAMEBUFFER_NO_MSAA].framebuffer.get();
        Texture2D* texture_color = msaa ? framebuffers[FRAMEBUFFER_MSAA].texture_color.get() : framebuffers[FRAMEBUFFER_NO_MSAA].texture_color.get();
        
        framebuffer->Bind();

        glEnable(GL_STENCIL_TEST);
        glViewport(0, 0, fb_width, fb_height);
        glClearColor(camera->background.r, camera->background.g, camera->background.b, camera->background.a);
        

        glStencilMask(0x01);

        //glClear(/*GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT*/);

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0x00);
        glStencilFunc(GL_ALWAYS, 0, 0XFF);

        App->renderer->Draw(camera, culling, framebuffer, fb_width, fb_height);
        App->debug_draw->Draw(camera, framebuffer->Id(), fb_width, fb_height);

        App->renderer->GetPostprocess()->Execute(texture_color, App->renderer->GetGBufferExportPass()->getDepth(), framebuffers[FRAMEBUFFER_POSTPROCESS].framebuffer.get(), fb_width, fb_height);

        if (std::get<bool>(App->hints->GetDHint(std::string("Enable Fxaa"), true)))
        {
            App->renderer->GetFxaaPass()->execute(framebuffers[FRAMEBUFFER_POSTPROCESS].texture_color.get(), fb_width, fb_height);
        }

        ShowTexture();

        DrawGuizmo(camera);

    }
    ImGui::EndChild();
}

void SceneViewport::PickSelection(ComponentCamera* camera, int mouse_x, int mouse_y)
{
    selection_buffer.framebuffer->Bind();
    glViewport(0, 0, fb_width, fb_height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    App->renderer->DrawForSelection(camera);

    float selection_value;
    glReadPixels(mouse_x, fb_height-mouse_y, 1, 1, GL_RED, GL_FLOAT, &selection_value);
    unsigned uid = *((unsigned*)&selection_value);

    App->editor->SetSelected(App->level->Find(uid));
}

void SceneViewport::DrawSelection(ComponentCamera* camera, Framebuffer* framebuffer)
{
    GameObject* const * selection = std::get_if<GameObject*>(&App->editor->GetSelection()); // App->editor->selection_type == ModuleEditor::SelectionGameObject ? App->editor->selected.go : nullptr;

    if(selection && *selection)
    {
        std::vector<Component*> components;
        (*selection)->FindComponents(Component::MeshRenderer, components);
        for(Component* comp : components)
        {
            ComponentMeshRenderer* mesh = static_cast<ComponentMeshRenderer*>(comp);

            framebuffer->Bind();
            float4x4 proj   = camera->GetProjectionMatrix();	
            float4x4 view   = camera->GetViewMatrix();
            glStencilMask(0XFF);
            glStencilFunc(GL_ALWAYS, 1, 0XFF);
            glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE);
            glDepthFunc(GL_LESS);

            App->programs->UseProgram("color", 0);

            float4 no_color(0.0, 0.0, 0.0, 0.0);

            float4x4 transform = std::get<GameObject*>(App->editor->GetSelection())->GetGlobalTransformation();
            glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
            glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
            glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));
            glUniform4fv(App->programs->GetUniformLocation("color"), 1, (float*)&no_color);

            ResourceMesh* mesh_res = mesh->GetMeshRes();

            if (mesh_res)
            {
                //mesh_res->UpdateUniforms(mesh->UpdateSkinPalette(), mesh->GetMorphTargetWeights());
                mesh_res->Draw();
            }

            float4 selection_color(1.0, 1.0, 0.0, 1.0);

            glUniform4fv(App->programs->GetUniformLocation("color"), 1, (float*)&selection_color);

            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
            glStencilMask(0xFF);
            glDisable(GL_DEPTH_TEST);

            glLineWidth(5);
            glPolygonMode(GL_FRONT, GL_LINE);

            if (mesh_res)
            {
                //mesh_res->UpdateUniforms(mesh->UpdateSkinPalette(), mesh->GetMorphTargetWeights());
                mesh_res->Draw();
            }

            glPolygonMode(GL_FRONT, GL_FILL);
            glEnable(GL_DEPTH_TEST);
            glLineWidth(1);
            App->programs->UnuseProgram();
        }
    }

}

void SceneViewport::ShowTexture()
{
    ImVec2 screenPos = ImGui::GetCursorScreenPos();
    
    void *id;
    if (std::get<bool>(App->hints->GetDHint(std::string("Enable Fxaa"), true)))
    {
        id = (void *)size_t(App->renderer->GetFxaaPass()->getOutput()->Id());
    }
    else
    {
        id = (void*)size_t(framebuffers[FRAMEBUFFER_POSTPROCESS].texture_color->Id());
    }

    ImGui::GetWindowDrawList()->AddImage(
            id,
            ImVec2(screenPos),
            ImVec2(screenPos.x + fb_width, screenPos.y + fb_height), 
            ImVec2(0, 1), ImVec2(1, 0));

    if (App->hints->GetBoolValue(ModuleHints::ENABLE_SHADOW_MAPPING) && App->hints->GetBoolValue(ModuleHints::SHOW_SHADOW_MAP))
    {
        if (App->hints->GetBoolValue(ModuleHints::ENABLE_CASCADE_SHADOW))
        {
            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)size_t(App->renderer->GetCascadeShadowPass()->getDepthTex(0)->Id()),
                ImVec2(screenPos),
                ImVec2(screenPos.x + fb_width * 0.2f, screenPos.y + fb_height * 0.2f),
                ImVec2(0, 1), ImVec2(1, 0));

            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)size_t(App->renderer->GetCascadeShadowPass()->getDepthTex(1)->Id()),
                ImVec2(screenPos.x + fb_width * 0.2f, screenPos.y),
                ImVec2(screenPos.x + fb_width * 0.4f, screenPos.y + fb_height * 0.2f),
                ImVec2(0, 1), ImVec2(1, 0));

            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)size_t(App->renderer->GetCascadeShadowPass()->getDepthTex(2)->Id()),
                ImVec2(screenPos.x + fb_width * 0.4f, screenPos.y),
                ImVec2(screenPos.x + fb_width * 0.6f, screenPos.y + fb_height * 0.2f),
                ImVec2(0, 1), ImVec2(1, 0));
        }
        else
        {
            ImGui::GetWindowDrawList()->AddImage(
                (ImTextureID)size_t(App->renderer->GetShadowmapPass()->getDepthTex()->Id()),
                ImVec2(screenPos),
                ImVec2(screenPos.x + fb_width * 0.4f, screenPos.y + fb_height * 0.4f),
                ImVec2(0, 1), ImVec2(1, 0));
        }

    }
    else
    {
        
        /*
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)App->renderer->GetDepthPrepass()->getPositionTexture()->Id(),
            ImVec2(screenPos),
            ImVec2(screenPos.x + fb_width*0.4f, screenPos.y + fb_height*0.4f),
            ImVec2(0, 1), ImVec2(1, 0));
            
        
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)App->renderer->GetScreenSpaceAO()->getResult()->Id(),
            ImVec2(screenPos),
            ImVec2(screenPos.x + fb_width , screenPos.y + fb_height ),
            ImVec2(0, 1), ImVec2(1, 0));
            */
    }

}

void SceneViewport::Save(Config* config) const
{
    config->AddBool("Draw Plane", draw_plane);
    config->AddBool("Draw Axis", draw_axis);
    config->AddBool("Debug Draw", debug_draw);
}

void SceneViewport::Load(Config* config)
{
	draw_plane = config->GetBool("Draw Plane", true);
	draw_axis = config->GetBool("Draw Axis", true);
	debug_draw = config->GetBool("Debug Draw", true);
}

void SceneViewport::GenerateFBO(FramebufferInfo& buffer, unsigned w, unsigned h, bool depth, bool msaa, bool hdr)
{
    buffer.framebuffer = std::make_unique<Framebuffer>(); 

    if(msaa)
    {
        buffer.texture_color = std::make_unique<Texture2D>(4, w, h, hdr ? GL_RGBA16F : GL_RGBA, true);
    }
    else
    {
        buffer.texture_color = std::make_unique<Texture2D>(w, h, hdr ? GL_RGBA16F : GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, nullptr, false);
    }

    buffer.framebuffer->AttachColor(buffer.texture_color.get());

    if(depth)
    {
       
        if(msaa)
        {
            buffer.texture_depth = std::make_unique<Texture2D>(4, w, h, GL_DEPTH24_STENCIL8, true);
        }
        else
        {
            buffer.texture_depth = std::make_unique<Texture2D>(w, h, GL_DEPTH24_STENCIL8, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);
        }
      

        buffer.framebuffer->AttachDepthStencil(buffer.texture_depth.get(), GL_DEPTH_ATTACHMENT);
    }

    assert(buffer.framebuffer->Check() == GL_FRAMEBUFFER_COMPLETE);
}

void SceneViewport::GenerateFBOs(unsigned w, unsigned h)
{
    if(w != fb_width || h != fb_height )
    {
        GenerateFBO(framebuffers[FRAMEBUFFER_NO_MSAA], w, h, true, false, true);
        GenerateFBO(framebuffers[FRAMEBUFFER_MSAA], w, h, true, true, true);
        GenerateFBO(framebuffers[FRAMEBUFFER_POSTPROCESS], w, h, false, false, false);

        selection_buffer.framebuffer   = std::make_unique<Framebuffer>(); 
        selection_buffer.texture_color = std::make_unique<Texture2D>(w, h, GL_R32F, GL_RED, GL_FLOAT, nullptr, false);
        selection_buffer.texture_depth = std::make_unique<Texture2D>(w, h, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr, false);

        selection_buffer.framebuffer->AttachColor(selection_buffer.texture_color.get());
		selection_buffer.framebuffer->AttachDepthStencil(selection_buffer.texture_depth.get(), GL_DEPTH_ATTACHMENT);

        assert(selection_buffer.framebuffer->Check() == GL_FRAMEBUFFER_COMPLETE);

        fb_width = w;
        fb_height = h;
    }
}

void SceneViewport::DrawQuickBar(ComponentCamera* camera)
{
    Application::State state = App->GetState();

    if(ImGui::BeginChild("ToolCanvas", ImVec2(405, 38), true, ImGuiWindowFlags_NoMove))
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

        ImGui::SameLine(150.f);
        ImGui::Checkbox("Grid", &draw_plane);
        ImGui::SameLine();
        ImGui::Checkbox("Axis", &draw_axis);
        ImGui::SameLine();
        ImGui::Checkbox("Dbg Draw", &debug_draw);

        ImGui::SameLine();
        ImGui::ColorEdit3("Bg", (float*)&camera->background, ImGuiColorEditFlags_NoInputs);
    }
    ImGui::EndChild();

	ImGui::SameLine();
    if(ImGui::BeginChild("ScaleCanvas", ImVec2(145, 38), true, ImGuiWindowFlags_NoMove))
    {
        float metric_proportion = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);
        float prev = metric_proportion;
        if(ImGui::DragFloat("Scale", &metric_proportion) && metric_proportion > 0.0f)
        {
            App->hints->SetFloatValue(ModuleHints::METRIC_PROPORTION, metric_proportion);
            ComponentCamera* camera = App->camera->GetDummy();

            if(prev > 0.0f)
            {
                float adapt = (metric_proportion/prev);
                camera->frustum.pos *= adapt;
                camera->frustum.nearPlaneDistance *= adapt;
                camera->frustum.farPlaneDistance *= adapt;
            }
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

void SceneViewport::DrawGuizmoProperties(GameObject* go) 
{
    bool local = guizmo_mode == ImGuizmo::LOCAL && guizmo_op != ImGuizmo::SCALE;

    float4x4 model = local ? go->GetLocalTransform() : go->GetGlobalTransformation();
    model.Transpose();

    ImGui::RadioButton("Translate", (int*)&guizmo_op, (int)ImGuizmo::TRANSLATE);
    ImGui::SameLine();
    ImGui::RadioButton("Rotate", (int*)&guizmo_op, (int)ImGuizmo::ROTATE);
    ImGui::SameLine();
    ImGui::RadioButton("Scale", (int*)&guizmo_op, (int)ImGuizmo::SCALE);

    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents((float*)&model, matrixTranslation, matrixRotation, matrixScale);
    bool transform_changed = ImGui::DragFloat3("Tr", matrixTranslation, 3);
    transform_changed = transform_changed || ImGui::DragFloat3("Rt", matrixRotation, 3);
    transform_changed = transform_changed || ImGui::DragFloat3("Sc", matrixScale, 3);

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

    switch (guizmo_op    )
    {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &guizmo_snap.x);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &guizmo_snap.x);
            break;
    }

}

void SceneViewport::DrawGuizmoProperties(PointLight* point) 
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

void SceneViewport::DrawGuizmoProperties(SpotLight* spot) 
{
    float4x4 model = float4x4::LookAt(spot->GetPosition(), spot->GetPosition()+spot->GetDirection(), float3::unitZ, float3::unitY, float3::unitY);
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

        spot->SetPosition(model.Row3(3));
        spot->SetDirection(model.Row3(2));
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
    }
}

void SceneViewport::DrawGuizmo(ComponentCamera* camera)
{
    std::visit([this, camera](auto ptr) {DrawGuizmo(camera, ptr); }, App->editor->GetSelection());
}

void SceneViewport::DrawGuizmo(ComponentCamera* camera, AmbientLight* light)
{
    // Intentionally blank
}


void SceneViewport::DrawGuizmo(ComponentCamera* camera, DirLight* dir_light)
{
    // Intentionally blank
}

void SceneViewport::DrawGuizmo(ComponentCamera* camera, Skybox* skybox)
{

}

void SceneViewport::DrawGuizmo(ComponentCamera* camera, GameObject* go)
{
    if (go)
    {
        ComponentCamera* go_camera = static_cast<ComponentCamera*>(go->FindFirstComponent(Component::Camera));

        float4x4 view = camera->GetOpenGLViewMatrix();
        float4x4 proj = camera->GetOpenGLProjectionMatrix();

        ImGuizmo::BeginFrame();
        ImGuizmo::Enable(true);

        float4x4 model = go->GetGlobalTransformation();
        model.Transpose();

        float4x4 delta;

        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(float(ImGui::GetCursorScreenPos().x), float(ImGui::GetCursorScreenPos().y), float(fb_width), float(fb_height));
        ImGuizmo::SetDrawlist();
        ImGuizmo::Manipulate((const float*)&view, (const float*)&proj, guizmo_op, guizmo_mode, (float*)&model, (float*)&delta, guizmo_useSnap ? &guizmo_snap.x : NULL);

        if (ImGuizmo::IsUsing() && !delta.IsIdentity())
        {
            model.Transpose();
            if (guizmo_mode == ImGuizmo::LOCAL || go->GetParent() == nullptr)
            {
                go->SetLocalTransform(model);
            }
            else
            {
                float4x4 parent = go->GetParent()->GetGlobalTransformation();
                parent.InverseOrthonormal();
                go->SetLocalTransform(parent * model);
            }

            App->level->GetRoot()->RecursiveCalcGlobalTransform(float4x4::identity, false);

            if (go_camera)
            {
                go_camera->OnUpdateTransform();
            }
        }


        float3 points[8];
        go->global_bbox.GetCornerPoints(points);
        std::swap(points[2], points[5]);
        std::swap(points[3], points[4]);
        std::swap(points[4], points[5]);
        std::swap(points[6], points[7]);
        dd::box(points, dd::colors::Yellow);

        if (go_camera)
        {
            float4x4 go_view = go_camera->GetViewMatrix();
            float4x4 go_proj = go_camera->GetProjectionMatrix();

            float4x4 inv_clip = go_proj * go_view;
            inv_clip.Inverse();

            dd::frustum(inv_clip, dd::colors::Gray);
        }
    }
}

void SceneViewport::DrawGuizmo(ComponentCamera* camera, PointLight* point)
{
    float4x4 view = camera->GetOpenGLViewMatrix();
    float4x4 proj = camera->GetOpenGLProjectionMatrix();

    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);

    float4x4 model = float4x4::identity;
    model.SetTranslatePart(point->GetPosition());
    model.Transpose();

    float4x4 delta;

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(float(ImGui::GetCursorScreenPos().x), float(ImGui::GetCursorScreenPos().y), float(fb_width), float(fb_height));
    ImGuizmo::SetDrawlist();
    ImGuizmo::Manipulate((const float*)&view, (const float*)&proj, guizmo_op, guizmo_mode, (float*)&model, (float*)&delta, guizmo_useSnap ? &guizmo_snap.x : NULL);

    model.Transpose();

    if (ImGuizmo::IsUsing() && !delta.IsIdentity())
    {
        point->SetPosition(model.TranslatePart());
    }

    float distance = point->GetRadius();

    float3 pos      = point->GetPosition();
    float3 color    = point->GetColor();
    float3 x_axis   = model.Col3(0);
    float3 y_axis   = model.Col3(1);
    float3 z_axis   = model.Col3(2);
    float3 xy_axis  = (x_axis+y_axis).Normalized();
    float3 xz_axis  = (x_axis+z_axis).Normalized();
    float3 yz_axis  = (y_axis+z_axis).Normalized();
    float3 xy_axis2 = (x_axis-y_axis).Normalized();
    float3 xz_axis2 = (x_axis-z_axis).Normalized();
    float3 yz_axis2 = (y_axis-z_axis).Normalized();
    float3 axis[]   = { x_axis, y_axis, z_axis, xy_axis, xz_axis, yz_axis,
                        xy_axis2, xz_axis2, yz_axis2 };


    for(uint i=0, count = sizeof(axis)/sizeof(float3); i < count; ++i)
    {
        dd::line(pos-axis[i]*distance, pos+axis[i]*distance, color);
    }
}

void SceneViewport::DrawGuizmo(ComponentCamera* camera, SpotLight* spot)
{
    float4x4 view = camera->GetOpenGLViewMatrix();
    float4x4 proj = camera->GetOpenGLProjectionMatrix();

    ImGuizmo::BeginFrame();
    ImGuizmo::Enable(true);

    float4x4 model = float4x4::LookAt(spot->GetPosition(), spot->GetPosition()+spot->GetDirection(), float3::unitZ, float3::unitY, float3::unitY);
    model.Transpose();

    float4x4 delta;

    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(float(ImGui::GetCursorScreenPos().x), float(ImGui::GetCursorScreenPos().y), float(fb_width), float(fb_height));
    ImGuizmo::SetDrawlist();
    ImGuizmo::Manipulate((const float*)&view, (const float*)&proj, guizmo_op, guizmo_mode, (float*)&model, (float*)&delta, guizmo_useSnap ? &guizmo_snap.x : NULL);

    if (ImGuizmo::IsUsing() && !delta.IsIdentity())
    {
        spot->SetPosition(model.Row3(3));
        spot->SetDirection(model.Row3(2));
    }

    float distance = spot->GetDistance();

    float3 pos   = spot->GetPosition();
    float3 dir   = spot->GetDirection();
    float3 color = spot->GetColor();
    float angle  = spot->GetOutterCutoff();
    float3 axis[] = { model.Row3(0), model.Row3(1), (model.Row3(0)+model.Row3(1)).Normalized(), (model.Row3(0)-model.Row3(1)).Normalized()};

    dd::arrow(pos, pos+dir*(distance*0.1f), color, distance*0.01f);
    dd::line(pos, pos+dir*distance, color);

    float tan_a = tanf(angle);

    for(uint i=0, count = sizeof(axis)/sizeof(float3); i < count; ++i)
    {
        dd::line(pos, pos+(dir+axis[i]*tan_a)*distance, color);
        dd::line(pos, pos+(dir*distance-axis[i]*tan_a)*distance, color);
    }
}

void SceneViewport::DrawGrid(ComponentCamera* camera)
{
    // bind matGeo and matVP
    App->programs->UseProgram("grid", 0);

    float4x4 proj = camera->GetProjectionMatrix();
    float4x4 view = camera->GetViewMatrix();

    float proportion   = App->hints->GetFloatValue(ModuleHints::METRIC_PROPORTION);
    float4x4 transform = float4x4::Scale(proportion, proportion, proportion);

    glUniformMatrix4fv(App->programs->GetUniformLocation("proj"), 1, GL_TRUE, reinterpret_cast<const float*>(&proj));
    glUniformMatrix4fv(App->programs->GetUniformLocation("view"), 1, GL_TRUE, reinterpret_cast<const float*>(&view));
    glUniformMatrix4fv(App->programs->GetUniformLocation("model"), 1, GL_TRUE, reinterpret_cast<const float*>(&transform));

    grid_vao->Bind();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    App->programs->UnuseProgram();
}
