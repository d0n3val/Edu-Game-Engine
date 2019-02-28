#include "PanelProperties.h"
#include "Application.h"
#include "Imgui/imgui.h"
#include "Imgui/imguizmo.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentMaterial.h"
#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"
#include "ComponentCamera.h"
#include "ComponentPath.h"
#include "ComponentRigidBody.h"
#include "ComponentSteering.h"
#include "ComponentMesh.h"
#include "ComponentAnimation.h"
#include "ModuleLevelManager.h"
#include "ModuleTextures.h"
#include "ModuleEditor.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditorCamera.h"
#include "ModuleHints.h"
#include "DebugDraw.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceStateMachine.h"
#include "ResourceAudio.h"
#include "ResourceAnimation.h"
#include "PanelResources.h"
#include "PanelGOTree.h"
#include "Viewport.h"
#include "SceneViewport.h"

#include "DirLight.h"
#include "AmbientLight.h"
#include "PointLight.h"
#include "SpotLight.h"

#include "OpenGL.h"

#include <list>

#include "mmgr/mmgr.h"

using namespace std;

// ---------------------------------------------------------
PanelProperties::PanelProperties() : Panel("Properties")
{
	width = default_width;
	height = default_height;
	posx = default_posx;
	posy = default_posy;
}

// ---------------------------------------------------------
PanelProperties::~PanelProperties()
{}

// ---------------------------------------------------------
void PanelProperties::Draw()
{
    //ImGui::Begin("Properties", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
    //if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None))
    {
        //if (ImGui::BeginTabItem("Selection"))
        {

            switch(App->editor->selection_type )
            {
                case ModuleEditor::SelectionGameObject:
                    if(App->editor->selected.go)
                        DrawGameObject(App->editor->selected.go);
                    break;
                case ModuleEditor::SelectionAmbientLight:
                    if(App->editor->selected.ambient)
                        DrawAmbientLight(App->editor->selected.ambient);
                    break;
                case ModuleEditor::SelectionDirLight:
                    if(App->editor->selected.directional)
                        DrawDirLight(App->editor->selected.directional);
                    break;
                case ModuleEditor::SelectionPointLight:
                    if(App->editor->selected.point)
                        DrawPointLight(App->editor->selected.point);
                    break;
                case ModuleEditor::SelectionSpotLight:
                    if(App->editor->selected.spot)
                        DrawSpotLight(App->editor->selected.spot);
                    break;
            }

            //ImGui::EndTabItem();
        }
        //ImGui::EndTabBar();
    }
    //ImGui::End();
}

// ---------------------------------------------------------
void PanelProperties::DrawAmbientLight(AmbientLight* light)
{
    if (ImGui::CollapsingHeader("Ambient light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }
    }
}

// ---------------------------------------------------------
void PanelProperties::DrawDirLight(DirLight* light)
{
    if (ImGui::CollapsingHeader("Directional light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        float azimuthal = light->GetAzimuthal();
        if(ImGui::SliderAngle("azimuthal", &azimuthal, 0.0f, 360.0f))
        {
            light->SetAzimuthal(azimuthal);
        }

        float polar = light->GetPolar();
        if(ImGui::SliderAngle("polar", &polar, 0.0f, 180.0f))
        {
            light->SetPolar(polar);
        }
    }
}

// ---------------------------------------------------------
void PanelProperties::DrawPointLight(PointLight* light)
{
    if (ImGui::CollapsingHeader("Point light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        ImGui::Separator();

        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);

        ImGui::Separator();

        float constant = light->GetConstantAtt();
        if(ImGui::InputFloat("constant att.", &constant, 0.00001f, 0.1f, "%.9f"))
        {
            light->SetConstantAtt(constant);
        }

        float linear = light->GetLinearAtt();
        if(ImGui::InputFloat("linear att.", &linear, 0.00001f, 0.1f, "%.9f"))
        {
            light->SetLinearAtt(linear);
        }

        float quadric = light->GetQuadricAtt();
        if(ImGui::InputFloat("quadric att.", &quadric, 0.00001f, 0.1f, "%.9f"))
        {
            light->SetQuadricAtt(quadric);
        }

        bool enabled = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enabled))
        {
            light->SetEnabled(enabled);
        }
    }
}

// ---------------------------------------------------------
void PanelProperties::DrawSpotLight(SpotLight* light)
{
    if (ImGui::CollapsingHeader("Spot light", ImGuiTreeNodeFlags_DefaultOpen))
    {
        float3 color = light->GetColor();
        if(ImGui::ColorEdit3("color", (float*)&color))
        {
            light->SetColor(color);
        }

        ImGui::Separator();

        App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(light);

        ImGui::Separator();

        float inner = light->GetInnerCutoff();
        if(ImGui::SliderAngle("inner cutoff", &inner, 0.0f, 90.0f))
        {
            light->SetInnerCutoff(inner);
        }

        float outter = light->GetOutterCutoff();
        if(ImGui::SliderAngle("outter cutoff", &outter, 0.0f, 90.0f))
        {
            light->SetOutterCutoff(outter);
        }

        ImGui::Separator();

        float constant = light->GetConstantAtt();
        if(ImGui::InputFloat("constant att.", &constant, 0.00001f, 0.1f, "%.9f"))
        {
            light->SetConstantAtt(constant);
        }

        float linear = light->GetLinearAtt();
        if(ImGui::InputFloat("linear att.", &linear, 0.00001f, 0.1f, "%.9f"))
        {
            light->SetLinearAtt(linear);
        }

        float quadric = light->GetQuadricAtt();
        if(ImGui::InputFloat("quadric att.", &quadric, 0.00001f, 0.1f, "%.9f"))
        {
            light->SetQuadricAtt(quadric);
        }

        bool enabled = light->GetEnabled();
        if(ImGui::Checkbox("Enabled", &enabled))
        {
            light->SetEnabled(enabled);
        }
    }
}

// ---------------------------------------------------------
void PanelProperties::DrawGameObject(GameObject* go)
{
    if (ImGui::BeginMenu("Options"))
    {
        if (ImGui::MenuItem("Reset Transform", nullptr, nullptr, (go != nullptr)))
        {
            go->SetLocalPosition(float3::zero);
            go->SetLocalScale(float3::one);
            go->SetLocalRotation(Quat::identity);
        }

        static_assert(Component::Types::Unknown == 9, "code needs update");
        if (ImGui::BeginMenu("New Component", (go != nullptr)))
        {
            if (ImGui::MenuItem("Audio Listener"))
                go->CreateComponent(Component::Types::AudioListener);
            if (ImGui::MenuItem("Audio Source"))
                go->CreateComponent(Component::Types::AudioSource);
            if (ImGui::MenuItem("Mesh"))
                go->CreateComponent(Component::Types::Mesh);
            if (ImGui::MenuItem("Material"))
                go->CreateComponent(Component::Types::Material);
            if (ImGui::MenuItem("Camera"))
                go->CreateComponent(Component::Types::Camera);
            if (ImGui::MenuItem("RigidBody"))
                go->CreateComponent(Component::Types::RigidBody);
            if (ImGui::MenuItem("Animation"))
                go->CreateComponent(Component::Types::Animation);
            if (ImGui::MenuItem("Steering"))
                go->CreateComponent(Component::Types::Steering);
            if (ImGui::MenuItem("Path"))
                go->CreateComponent(Component::Types::Path);
            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (go != nullptr )
    {

        // Active check box
        bool active = go->IsActive();
        ImGui::Checkbox(" ", &active);
        go->SetActive(active);

        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("UID: %u", go->GetUID());

        ImGui::SameLine();

        // Text Input for the name
        char name[50];
        strcpy_s(name, 50, go->name.c_str());
        if (ImGui::InputText("Name", name, 50,
                    ImGuiInputTextFlags_AutoSelectAll |
                    ImGuiInputTextFlags_EnterReturnsTrue))
            go->name = name;

        // Transform section ============================================
        if (ImGui::CollapsingHeader("Local Transformation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            App->renderer3D->viewport->GetScene()->DrawGuizmoProperties(go);

            ImGui::Text("Bounding Box: ");
            ImGui::SameLine();
            if (go->global_bbox.IsFinite())
            {
                float3 size = go->GetLocalBBox().Size();
                ImGui::TextColored(IMGUI_YELLOW, "%.2f %.2f %.2f", size.x, size.y, size.x);
            }
            else
                ImGui::TextColored(IMGUI_YELLOW, "- not generated -");

            ImGui::Text("Velocity: ");
            ImGui::SameLine();
            float3 vel = go->GetVelocity();
            ImGui::TextColored(IMGUI_YELLOW, "%.2f %.2f %.2f (%.2f m/s)", vel.x, vel.y, vel.x, vel.Length());
        }

        // Iterate all components and draw
        static_assert(Component::Types::Unknown == 9, "code needs update");
        for (list<Component*>::iterator it = go->components.begin(); it != go->components.end(); ++it)
        {
            ImGui::PushID(*it);
            if (InitComponentDraw(*it, (*it)->GetTypeStr()))
            {

                switch ((*it)->GetType())
                {
                    case Component::Types::Mesh:
                        DrawMeshComponent((ComponentMesh*)(*it));
                        break;
                    case Component::Types::Material:
                        DrawMaterialComponent((ComponentMaterial*)(*it));
                        break;
                    case Component::Types::AudioSource:
                        DrawAudioSourceComponent((ComponentAudioSource*)(*it));
                        break;
                    case Component::Types::AudioListener:
                        DrawAudioListenerComponent((ComponentAudioListener*)(*it));
                        break;
                    case Component::Types::Camera:
                        DrawCameraComponent((ComponentCamera*)(*it));
                        break;
                    case Component::Types::RigidBody:
                        ((ComponentRigidBody*)(*it))->DrawEditor();
                        break;
                    case Component::Types::Animation:
                        DrawAnimationComponent(static_cast<ComponentAnimation*>(*it));
                        break;
                    case Component::Types::Steering:
                        ((ComponentSteering*)(*it))->DrawEditor();
                        break;
                    case Component::Types::Path:
                        ((ComponentPath*)(*it))->DrawEditor();
                        break;
                }
            }
            ImGui::PopID();
        }

    }

}

UID PanelProperties::PickResource(UID resource, int type)
{
	UID ret = 0;

	if (resource > 0)
	{
		const Resource* res = App->resources->Get(resource);

		std::string file;
		ImGui::TextColored(IMGUI_YELLOW, "%s", res->GetFile());
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Type: %s", res->GetTypeStr());
			ImGui::Text("UID: %llu", res->GetUID());
			ImGui::Text("Lib: %s", res->GetExportedFile());
			ImGui::EndTooltip();
		}
		if (ImGui::Button("Change Resource"))
			ImGui::OpenPopup("Load Resource");
	}
	else
	{
		if (ImGui::Button("Attach Resource"))
			ImGui::OpenPopup("Load Resource");
	}

	if (ImGui::BeginPopup("Load Resource"))
	{
			UID r = 0;
			r = App->editor->res->DrawResourceType((Resource::Type) type);
			ret = (r) ? r : ret;
		ImGui::EndPopup();
	}

	return ret;
}

UID PanelProperties::PickResourceModal(int type)
{
    char tmp[128];
    sprintf_s(tmp, 127, "Select %s", Resource::GetTypeStr(Resource::Type(type)));

    if (ImGui::Button(tmp))
    {
        ImGui::OpenPopup("Select");
    }

    return OpenResourceModal(type, "Select");
}

UID PanelProperties::OpenResourceModal(int type, const char* popup_name)
{
	UID new_res = 0;

    ImGui::SetNextWindowSize(ImVec2(420,300));
    if (ImGui::BeginPopupModal(popup_name, nullptr, ImGuiWindowFlags_NoResize))
    {
        if(ImGui::BeginChild("Canvas", ImVec2(400, 240), true, ImGuiWindowFlags_NoMove))
        {
            UID r = 0;
            r = App->editor->res->DrawResourceType(Resource::Type(type), true);

            if(r != 0)
            {
                ImGui::CloseCurrentPopup();
                new_res = r;
            }
        }
        ImGui::EndChild();

        ImGui::Indent(272);
        if(ImGui::Button("Close", ImVec2(128, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return new_res;
}

const GameObject* PanelProperties::PickGameObject(const GameObject* current) const
{
	const GameObject* ret = nullptr;

	ImGui::Text("Current: ");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%s", (current) ? current->name.c_str() : "- not assigned -");
	if (ImGui::Button("Pick Another"))
		ImGui::OpenPopup("Pick GameObject");

	if (ImGui::BeginPopup("Pick GameObject"))
	{
		// Draw the tree
		GameObject* root = App->level->GetRoot();
		for (list<GameObject*>::const_iterator it = root->childs.begin(); it != root->childs.end(); ++it)
			RecursiveDrawTree(*it, &ret);

		ImGui::EndPopup();
	}

	return ret;
}

void PanelProperties::RecursiveDrawTree(const GameObject * go, const GameObject** selected) const
{
	if (ImGui::TreeNodeEx(go, (go->childs.size() > 0) ? 0 : ImGuiTreeNodeFlags_Bullet, go->name.c_str())) 
	{
		if (ImGui::IsItemHoveredRect() && ImGui::IsMouseDoubleClicked(0))
		{
			*selected = go;
			ImGui::CloseCurrentPopup();
		}

		for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
			RecursiveDrawTree(*it, selected);

		ImGui::TreePop();
	}
	else
	{
		if (ImGui::IsItemHoveredRect() && ImGui::IsMouseDoubleClicked(0))
		{
			*selected = go;
			ImGui::CloseCurrentPopup();
		}
	}
}

bool PanelProperties::InitComponentDraw(Component* component, const char * name)
{
	bool ret = false;

	if (ImGui::CollapsingHeader(name, ImGuiTreeNodeFlags_DefaultOpen))
	{
		bool active = component->IsActive();
		if(ImGui::Checkbox("Active", &active))
			component->SetActive(active);
		ImGui::SameLine();
		if (ImGui::SmallButton("Delete Component"))
			component->flag_for_removal = true;
		ret = true;
	}

	return ret;
}

void PanelProperties::DrawMeshComponent(ComponentMesh * component)
{
    UID new_res = 0;

	const ResourceMesh* res = component->GetResource();

    if(res != nullptr)
    {
        ImGui::TextColored(IMGUI_YELLOW, "%s", res->GetFile());
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Type: %s", res->GetTypeStr());
            ImGui::Text("UID: %llu", res->GetUID());
            ImGui::Text("Lib: %s", res->GetExportedFile());
            ImGui::EndTooltip();
        }

        ImGui::TextColored(ImVec4(1,1,0,1), "%u Triangles (%u indices %u vertices)", res->num_indices / 3, res->num_indices, res->num_vertices); 

        char attributes[256];
        strcpy_s(attributes, "\nAttributes: \n\n\tPositions");
        if((res->attribs & ResourceMesh::ATTRIB_TEX_COORDS_0) != 0)
        {
            strcat_s(attributes, "\n\tTexCoords0");
        }
        if((res->attribs & ResourceMesh::ATTRIB_NORMALS) != 0)
        {
            strcat_s(attributes, "\n\tNormals");
        }
        if((res->attribs & ResourceMesh::ATTRIB_TANGENTS) != 0)
        {
            strcat_s(attributes, "\n\tTangents");
        }
        if((res->attribs & ResourceMesh::ATTRIB_BONES) != 0)
        {
            strcat_s(attributes, "\n\tBones");
        }
        strcat_s(attributes, "\n\n");

        ImGui::TextColored(ImVec4(1,1,0,1), attributes);

        bool visible = component->GetVisible();
        if(ImGui::Checkbox("Visible", &visible))
        {
            component->SetVisible(visible);
        }
    }

    new_res = PickResourceModal(Resource::mesh);

    if (new_res > 0)
    {
        component->SetResource(new_res);
        App->resources->Get(new_res)->LoadToMemory();
    }
}

void PanelProperties::DrawAudioSourceComponent(ComponentAudioSource * component)
{
	UID new_res = PickResource(component->GetResourceUID(), Resource::audio);
	if (new_res > 0)
		component->SetResource(new_res);

	const ResourceAudio* res = (const ResourceAudio*) component->GetResource();
	const char* file = (res) ? res->GetFile() : nullptr;

	ImGui::Text("File: ");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, (file) ? file : "No file loaded");

	ImGui::Text("Format: ");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, (res) ? res->GetFormatStr() : "???");
	ImGui::SameLine();
	ImGui::Checkbox("Is 2D", &component->is_2d);

	ImGui::SliderFloat("Fade In", (float*)&component->fade_in, 0.0f, 10.0f);
	ImGui::SliderFloat("Fade Out", (float*)&component->fade_out, 0.0f, 10.0f);
	ImGui::DragFloat("Min Distance", (float*)&component->min_distance, 0.1f, 0.1f, 10000.0f);
	ImGui::DragFloat("Max Distance", (float*)&component->max_distance, 0.1f, 0.1f, 10000.0f);
	ImGui::SliderInt("Cone In", (int*)&component->cone_angle_in, 0, 360);
	ImGui::SliderInt("Cone Out", (int*)&component->cone_angle_out, 0, 360);
	ImGui::SliderFloat("Vol Out Cone", (float*)&component->out_cone_vol, 0.0f, 1.0f);
	
	static const char * states[] = { 
		"Not Loaded", 
		"Stopped",
		"About to Play",
		"Playing",
		"About to Pause",
		"Pause",
		"About to Unpause",
		"About to Stop"
	};

	ImGui::Text("Current State: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", states[component->GetCurrentState()]);

	if (ImGui::Button("Play"))
		component->Play();

	ImGui::SameLine();
	if (ImGui::Button("Pause"))
		component->Pause();

	ImGui::SameLine();
	if (ImGui::Button("Unpause"))
		component->UnPause();

	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		component->Stop();
}

void PanelProperties::DrawAudioListenerComponent(ComponentAudioListener * component)
{
	ImGui::DragFloat("Distance", (float*)&component->distance, 0.1f, 0.1f, 10000.0f);
	ImGui::SliderFloat("Roll Off", (float*)&component->roll_off, 0.0f, 10.0f);
	ImGui::SliderFloat("Doppler", (float*)&component->doppler, 0.0f, 10.0f);
}

void PanelProperties::DrawCameraComponent(ComponentCamera * component)
{
	ImGui::Checkbox("Frustum Culling", &component->frustum_culling);

	float near_plane = component->GetNearPlaneDist();
	if (ImGui::DragFloat("Near Plane", &near_plane, 0.1f, 0.1f, 1000.0f))
		component->SetNearPlaneDist(near_plane);

	float far_plane = component->GetFarPlaneDist();
	if (ImGui::DragFloat("Far Plane", &far_plane, 10.0f, 25.f, 10000.0f))
		component->SetFarPlaneDist(far_plane);

	float fov = component->GetFOV();
	if (ImGui::SliderFloat("Field of View", &fov, 1.0f, 179.0f))
		component->SetFOV(fov);

	float aspect_ratio = component->GetAspectRatio();
	if (ImGui::DragFloat("Aspect Ratio", &aspect_ratio, 0.1f, 0.1f, 10000.0f))
		component->SetAspectRatio(aspect_ratio);

	ImGui::ColorEdit3("Background", &component->background);

	const GameObject* go = PickGameObject(component->looking_at);
	if (go != nullptr)
		component->looking_at = go;

	bool is_active = App->renderer3D->active_camera == component;
	if (ImGui::Checkbox("Is Active Camera", &is_active))
	{
		if(is_active == true)
			App->renderer3D->active_camera = component;
		else
			App->renderer3D->active_camera = App->camera->GetDummy();
	}
}

void PanelProperties::DrawMaterialComponent(ComponentMaterial* component)
{
    ResourceMaterial* mat_res = component->GetResource();

    UID new_res = PickResourceModal(Resource::material);

    ImGui::SameLine();
    if(ImGui::Button("New material"))
    {
        ResourceMaterial* material = static_cast<ResourceMaterial*>(App->resources->CreateNewResource(Resource::material, 0));

        bool save_ok = material->Save();

        if(save_ok)
        {
            new_res = material->GetUID();
        }
    }

    ImGui::Separator();

    const char* names[ComponentMaterial::RENDER_COUNT] = { "Opaque", "Transparent" };

    int index = int(component->RenderMode());
    if(ImGui::Combo("Render mode", &index, names, int(ComponentMaterial::RENDER_COUNT)))
    {
		component->SetRenderMode(ComponentMaterial::ERenderMode(index));
    }

    ImGui::Separator();

    bool debug_draw = component->GetDDTangent();
    if(ImGui::Checkbox("Debug draw tangent space", &debug_draw))
    {
        component->SetDDTangent(debug_draw);
    }

    ImGui::Separator();

    if(new_res > 0)
    {
		component->SetResource(new_res);
    }
    else
    {
        if(mat_res)
        {
            bool modified = false;

            if (ImGui::CollapsingHeader("Ambient", ImGuiTreeNodeFlags_DefaultOpen))
            {
                modified = TextureButton(mat_res, ResourceMaterial::TextureOcclusion, "Occlusion");
                float k_ambient = mat_res->GetKAmbient();
                if(ImGui::SliderFloat("k ambient", &k_ambient, 0.0f, 1.0f))
                {
                    mat_res->SetKAmbient(k_ambient);
                    modified = true;
                }
            }

            if(ImGui::CollapsingHeader("Diffuse", ImGuiTreeNodeFlags_DefaultOpen))
            {
                modified = TextureButton(mat_res, ResourceMaterial::TextureDiffuse, "Diffuse") || modified;
                float4 color = mat_res->GetDiffuseColor();
                ImGui::PushID("diffuse");
                if(ImGui::ColorEdit4("color", (float*)&color))
                {
                    mat_res->SetDiffuseColor(color);
                    modified = true;
                }
                ImGui::PopID();

                float k_diffuse = mat_res->GetKDiffuse();
                if(ImGui::SliderFloat("k diffuse", &k_diffuse, 0.0f, 1.0f))
                {
                    mat_res->SetKDiffuse(k_diffuse);
                    modified = true;
                }
            }

            if(ImGui::CollapsingHeader("Specular", ImGuiTreeNodeFlags_DefaultOpen))
            {
                modified = TextureButton(mat_res, ResourceMaterial::TextureSpecular, "Specular") || modified;
                float3 color = mat_res->GetSpecularColor();
                ImGui::PushID("specular");
                if(ImGui::ColorEdit3("color", (float*)&color))
                {
                    mat_res->SetSpecularColor(color);
                    modified = true;
                }
                ImGui::PopID();

                float k_specular = mat_res->GetKSpecular();
                if(ImGui::SliderFloat("k specular", &k_specular, 0.0f, 1.0f))
                {
                    mat_res->SetKSpecular(k_specular);
                    modified = true;
                }

                float shininess = min(max(mat_res->GetShininess(), 0.0f), 1.0f);
                if(ImGui::SliderFloat("Shininess", &shininess, 0.0f, 1.0f))
                {
                    mat_res->SetShininess(shininess);
                    modified = true;
                }
            }

            if(ImGui::CollapsingHeader("Normal", ImGuiTreeNodeFlags_DefaultOpen))
            {
                modified = TextureButton(mat_res, ResourceMaterial::TextureNormal, "Normal") || modified;
            }

            if(ImGui::CollapsingHeader("Emissive", ImGuiTreeNodeFlags_DefaultOpen))
            {
                modified = TextureButton(mat_res, ResourceMaterial::TextureEmissive, "Emissive") || modified;
                float3 color = mat_res->GetEmissiveColor();
                ImGui::PushID("emissive");
                if(ImGui::ColorEdit3("color", (float*)&color))
                {
                    mat_res->SetEmissiveColor(color);
                    modified = true;
                }
                ImGui::PopID();

            }


            if(modified)
            {
                mat_res->Save();
            }

        }
    }
}

bool PanelProperties::TextureButton(ResourceMaterial* material, uint texture, const char* name)
{
    bool modified = false;
    ResourceTexture* info = material->GetTextureRes(ResourceMaterial::Texture(texture));

    ImVec2 size(64.0f, 64.0f);

    if (info != nullptr)
    {
		ImGui::PushID(texture);
        glEnable(GL_FRAMEBUFFER_SRGB);  
        if(ImGui::ImageButton((ImTextureID) info->GetID(), size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128)))
        {
			ImGui::PopID();
			ImGui::OpenPopup(name);
        }
        else 
        {
			ImGui::PopID();

			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("%s", info->GetFile());
			}
        }
        glDisable(GL_FRAMEBUFFER_SRGB);  
    }
    else
    {
		ImGui::PushID(texture);
        if(ImGui::ImageButton((ImTextureID) 0, size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128)))
        {
			ImGui::PopID();
			ImGui::OpenPopup(name);
        }
		else
		{
			ImGui::PopID();
		}

    }

    ImGui::SameLine();
    ImGui::BeginGroup();
    ImGui::Text("Texture:");
    if(info != nullptr)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IMGUI_YELLOW);

		std::string file;
		App->fs->SplitFilePath(info->GetFile(), nullptr, &file);

        ImGui::Text("%s", file.c_str());
        ImGui::Text("(%u,%u) %s %u bpp %s", info->GetWidth(), info->GetHeight(), info->GetFormatStr(), info->GetBPP(), info->GetCompressed() ? "compressed" : "");
        ImGui::PopStyleColor();

        ImGui::PushID(name);
        bool mips = info->HasMips();
        if(ImGui::Checkbox("Mipmaps", &mips))
        {
            info->EnableMips(mips);
        }
        ImGui::PopID();

        char tmp[128];
        sprintf_s(tmp, 127, "%sLinear", name);
        ImGui::PushID(tmp);
        bool linear = !info->GetLinear();
        if(ImGui::Checkbox("sRGB", &linear))
        {
            info->SetLinear(!linear);
        }
        ImGui::PopID();

        ImGui::PushID(name);
        if(ImGui::SmallButton("Delete"))
        {
            material->SetTexture(ResourceMaterial::Texture(texture), 0);
            modified = true;
        }
        ImGui::PopID();
    }
    ImGui::EndGroup();

    UID new_res = OpenResourceModal(Resource::texture, name);

    if(new_res != 0)
    {
        material->SetTexture(ResourceMaterial::Texture(texture), new_res);
        modified = true;
    }

    return modified;
}

void PanelProperties::DrawAnimationComponent(ComponentAnimation* component)
{
    ResourceStateMachine* state_res = component->GetResource();

    UID new_res = PickResourceModal(Resource::state_machine);

    ImGui::SameLine();
    if(ImGui::Button("New State machine"))
    {
        ResourceStateMachine* state_machine = static_cast<ResourceStateMachine*>(App->resources->CreateNewResource(Resource::state_machine, 0));

        bool save_ok = state_machine->Save();

        if(save_ok)
        {
            new_res = state_machine->GetUID();
        }
    }


    bool debug_draw = component->GetDebugDraw();
    if(ImGui::Checkbox("Debug draw", &debug_draw))
    {
        component->SetDebugDraw(debug_draw);
    }

    if(new_res > 0)
    {
		component->SetResource(new_res);
    }
    else if(state_res != nullptr)
    {
        char name[128];

        strcpy_s(name, state_res->GetName());

        if(ImGui::InputText("Resource name", name, 128))
        {
            state_res->SetName(name);
            state_res->Save();
        }

        if (ImGui::Button("Add clip"))
        {
            state_res->AddClip(HashString("noname"), 0, true);
            state_res->Save();
        }

        ImGui::Separator();

        uint i=0; 
        while(i < state_res->GetNumClips())
        {
            ResourceAnimation* resource = static_cast<ResourceAnimation*>(App->resources->Get(state_res->GetClipRes(i)));

            strcpy_s(name, state_res->GetClipName(i).C_str());

            ImGui::PushID(i);
            if(ImGui::InputText("Clip name", name, 128))
            {
                state_res->SetClipName(i, HashString(name));
                state_res->Save();
            }

            ImGui::LabelText("Resource", resource ? resource->GetName() : "Unknown");
            ImGui::SameLine();
            if(ImGui::ArrowButton("resource", ImGuiDir_Right))
            {
                ImGui::OpenPopup("Select");
            }

            UID new_res = OpenResourceModal(Resource::animation, "Select");


            if (new_res > 0)
            {
                state_res->SetClipRes(i, new_res);
                state_res->Save();

                App->resources->Get(new_res)->LoadToMemory();
            }

            bool loop = state_res->GetClipLoop(i);
            if(ImGui::Checkbox("Loop", &loop))
            {
                state_res->SetClipLoop(i, loop);
                state_res->Save();
            }

            ImGui::SameLine();
            if(ImGui::Button("Remove"))
            {
                state_res->RemoveClip(i);
                state_res->Save();
            }
            else
            {
                ++i;
            }

            ImGui::Separator();
            ImGui::PopID();
        }

        if(App->GetState() != Application::stop)
        {
            HashString active_node = component->GetActiveNode();

            for(uint i=0; i< state_res->GetNumTransitions(); ++i)
            {
                if(state_res->GetTransitionSource(i) == active_node)
                {
                    HashString trigger = state_res->GetTransitionTrigger(i);
                    if(trigger && ImGui::Button(trigger.C_str()))
                    {
                        component->SendTrigger(trigger);
                    }
                }
            }
        }
    }
}
