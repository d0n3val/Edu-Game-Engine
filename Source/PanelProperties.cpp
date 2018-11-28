#include "PanelProperties.h"
#include "Application.h"
#include "Imgui/imgui.h"
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
#include "ModuleMeshes.h"
#include "ModuleLevelManager.h"
#include "ModuleTextures.h"
#include "ModuleEditor.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditorCamera.h"
#include "DebugDraw.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ResourceMaterial.h"
#include "ResourceAudio.h"
#include "ResourceAnimation.h"
#include "PanelResources.h"
#include "PanelGOTree.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
PanelProperties::PanelProperties() : Panel("Properties", SDL_SCANCODE_3)
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
	GameObject* selected = App->editor->selected;
    ImGui::Begin("Properties", &active, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);

	if (ImGui::BeginMenu("Options"))
	{
		if (ImGui::MenuItem("Reset Transform", nullptr, nullptr, (selected != nullptr)))
		{
			selected->SetLocalPosition(float3::zero);
			selected->SetLocalScale(float3::one);
			selected->SetLocalRotation(Quat::identity);
		}

		static_assert(Component::Types::Unknown == 10, "code needs update");
        if (ImGui::BeginMenu("New Component", (selected != nullptr)))
        {
			if (ImGui::MenuItem("Audio Listener"))
				selected->CreateComponent(Component::Types::AudioListener);
			if (ImGui::MenuItem("Audio Source"))
				selected->CreateComponent(Component::Types::AudioSource);
			if (ImGui::MenuItem("Mesh"))
				selected->CreateComponent(Component::Types::Mesh);
			if (ImGui::MenuItem("Material"))
				selected->CreateComponent(Component::Types::Material);
			if (ImGui::MenuItem("Camera"))
				selected->CreateComponent(Component::Types::Camera);
			if (ImGui::MenuItem("RigidBody"))
				selected->CreateComponent(Component::Types::RigidBody);
			if (ImGui::MenuItem("Animation"))
				selected->CreateComponent(Component::Types::Animation);
			if (ImGui::MenuItem("Steering"))
				selected->CreateComponent(Component::Types::Steering);
			if (ImGui::MenuItem("Path"))
				selected->CreateComponent(Component::Types::Path);
            ImGui::EndMenu();
        }

		ImGui::EndMenu();
	}

	if (selected != nullptr )
	{

		// Active check box
		bool active = selected->IsActive();
		ImGui::Checkbox(" ", &active);
		selected->SetActive(active);

		if(ImGui::IsItemHovered())
			ImGui::SetTooltip("UID: %u", selected->GetUID());

		ImGui::SameLine();

		// Text Input for the name
		char name[50];
		strcpy_s(name, 50, selected->name.c_str());
		if (ImGui::InputText("", name, 50,
			ImGuiInputTextFlags_AutoSelectAll |
			ImGuiInputTextFlags_EnterReturnsTrue))
			selected->name = name;

		// Transform section ============================================
		if (ImGui::CollapsingHeader("Local Transformation", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float3 pos = selected->GetLocalPosition();
			float3 rot = selected->GetLocalRotation();
			rot.x = fabsf(rot.x);
			rot.y = fabsf(rot.y);
			rot.z = fabsf(rot.z);
			float3 scale = selected->GetLocalScale();;

			if (ImGui::DragFloat3("Position", (float*)&pos, 0.25f))
				selected->SetLocalPosition(pos);

			if (ImGui::SliderAngle3("Rotation", (float*)&rot))
				selected->SetLocalRotation(rot);

			if (ImGui::DragFloat3("Scale", (float*)&scale, 0.05f))
				selected->SetLocalScale(scale);

			ImGui::Text("Bounding Box: ");
			ImGui::SameLine();
			if (selected->global_bbox.IsFinite())
			{
				float3 size = selected->GetLocalBBox().Size();
				ImGui::TextColored(IMGUI_YELLOW, "%.2f %.2f %.2f", size.x, size.y, size.x);
			}
			else
				ImGui::TextColored(IMGUI_YELLOW, "- not generated -");

			ImGui::Text("Velocity: ");
			ImGui::SameLine();
			float3 vel = selected->GetVelocity();
			ImGui::TextColored(IMGUI_YELLOW, "%.2f %.2f %.2f (%.2f m/s)", vel.x, vel.y, vel.x, vel.Length());
		}

		// Iterate all components and draw
		static_assert(Component::Types::Unknown == 10, "code needs update");
		for (list<Component*>::iterator it = selected->components.begin(); it != selected->components.end(); ++it)
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

    ImGui::End();
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
    }

    if (ImGui::Button("Attach mesh"))
    {
        ImGui::OpenPopup("Select");
    }

    if (ImGui::BeginPopup("Select"))
    {
        UID r = 0;
        r = App->editor->res->DrawResourceType((Resource::Type::mesh));
        new_res = (r) ? r : new_res;
        ImGui::EndPopup();
    }

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

void PanelProperties::DrawMaterialComponent(ComponentMaterial * component)
{
    ResourceMaterial* mat_res = component->GetResource();

    ImGui::PushID(mat_res);
	UID new_res = PickResource(mat_res != nullptr ? mat_res->GetUID() : 0, Resource::material);
    ImGui::PopID();

    if(new_res > 0)
    {
		component->SetResource(new_res);
    }
    else
    {
        if(mat_res)
        {
            const char* texture_names[ResourceMaterial::TextureCount] = { "Diffuse", "Specular", "Normal", "Occlusion" };

            for(uint i=0; i< ResourceMaterial::TextureCount; ++i)
            {
                ImGui::PushID(i);
                ImGui::Separator();
                ImGui::BulletText(texture_names[i]);
                UID new_res = PickResource(mat_res->GetTexture(ResourceMaterial::Texture(i)), Resource::texture);
                ImGui::PopID();
                if (new_res != 0)
                {
                    mat_res->SetTexture(ResourceMaterial::Texture(i), new_res);
                    break;
                }

                const ResourceTexture* info = mat_res->GetTextureRes(ResourceMaterial::Texture(i));

                if (info != nullptr)
                {
                    ImGui::Text("(%u,%u) %0.1f Mb", info->GetWidth(), info->GetHeight(), info->GetBytes() / (1024.f*1024.f));
                    ImGui::Text("Format: %s Depth: %u Bpp: %u Mips: %u", info->GetFormatStr(), info->GetDepth(), info->GetBPP(), info->GetMips());

                    ImVec2 size((float)info->GetWidth(), (float)info->GetHeight());
                    float max_size = 64.f;

                    if (size.x > max_size || size.y > max_size)
                    {
                        if (size.x > size.y)
                        {
                            size.y *= max_size / size.x;
                            size.x = max_size;
                        }
                        else
                        {
                            size.x *= max_size / size.y;
                            size.y = max_size;
                        }
                    }

                    ImGui::Image((ImTextureID) info->GetID(), size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128));
                }
            }
        }
    }
}

