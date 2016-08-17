#include "PanelProperties.h"
#include "Application.h"
#include "Imgui/imgui.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentMesh.h"
#include "ComponentMaterial.h"
#include "ComponentAudioSource.h"
#include "ComponentAudioListener.h"
#include "ComponentCamera.h"
#include "ComponentBone.h"
#include "ComponentSkeleton.h"
#include "ComponentAnimation.h"
#include "ModuleMeshes.h"
#include "ModuleLevelManager.h"
#include "ModuleTextures.h"
#include "ModuleEditor.h"
#include "ModuleResources.h"
#include "ModuleFileSystem.h"
#include "DebugDraw.h"
#include "ResourceTexture.h"
#include "ResourceMesh.h"
#include "ResourceAudio.h"
#include "ResourceBone.h"
#include "ResourceAnimation.h"
#include "PanelResources.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
PanelProperties::PanelProperties() : Panel("Properties", SDL_SCANCODE_F3)
{
	width = 325;
	height = 578;
	posx = 956;
	posy = 21;
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
			selected->SetLocalRotation(float3::zero);
		}

        if (ImGui::BeginMenu("New Component", (selected != nullptr)))
        {
			if (ImGui::MenuItem("Audio Listener"))
				selected->CreateComponent(ComponentTypes::AudioListener);
			if (ImGui::MenuItem("Audio Source"))
				selected->CreateComponent(ComponentTypes::AudioSource);
			if (ImGui::MenuItem("Geometry"))
				selected->CreateComponent(ComponentTypes::Geometry);
			if (ImGui::MenuItem("Material"))
				selected->CreateComponent(ComponentTypes::Material);
			if (ImGui::MenuItem("Camera"))
				selected->CreateComponent(ComponentTypes::Camera);
			if (ImGui::MenuItem("Bone", nullptr, nullptr, false))
				selected->CreateComponent(ComponentTypes::Bone);
			if (ImGui::MenuItem("Skeleton"))
				selected->CreateComponent(ComponentTypes::Skeleton);
			if (ImGui::MenuItem("Animation"))
				selected->CreateComponent(ComponentTypes::Animation);
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
			float3 scale = selected->GetLocalScale();;

			if (ImGui::DragFloat3("Position", (float*)&pos, 0.25f))
				selected->SetLocalPosition(pos);

			if(ImGui::SliderFloat3("Rotation", (float*)&rot, -PI, PI))
				selected->SetLocalRotation(rot);

			if (ImGui::DragFloat3("Scale", (float*)&scale, 0.05f))
				selected->SetLocalScale(scale);

			DebugDraw(selected->GetGlobalTransformation());

			ImGui::Text("Bounding Box:");
			ImGui::SameLine();
			if (selected->global_bbox.IsFinite())
			{
				float3 size = selected->GetLocalBBox().Size();
				ImGui::TextColored(IMGUI_YELLOW, "%.2f %.2f %.2f", size.x, size.y, size.x);
			}
			else
				ImGui::TextColored(IMGUI_YELLOW, "- not generated -");
		}

		// Iterate all components and draw
		for (list<Component*>::iterator it = selected->components.begin(); it != selected->components.end(); ++it)
		{
			switch ((*it)->GetType())
			{
				case ComponentTypes::Geometry:
				{
					if(InitComponentDraw(*it, "Geometry Mesh"))
						DrawMeshComponent((ComponentMesh*)(*it));
				}	break;
				case ComponentTypes::Material:
				{
					if(InitComponentDraw(*it, "Material"))
						DrawMaterialComponent((ComponentMaterial*)(*it));
				}	break;
				case ComponentTypes::AudioSource:
				{
					if(InitComponentDraw(*it, "Audio Source"))
						DrawAudioSourceComponent((ComponentAudioSource*)(*it));
				}	break;
				case ComponentTypes::AudioListener:
				{
					if(InitComponentDraw(*it, "Audio Listener"))
						DrawAudioListenerComponent((ComponentAudioListener*)(*it));
				}	break;
				case ComponentTypes::Camera:
				{
					if(InitComponentDraw(*it, "Camera"))
						DrawCameraComponent((ComponentCamera*)(*it));
				}	break;
				case ComponentTypes::Bone:
				{
					if(InitComponentDraw(*it, "Bone"))
						DrawBoneComponent((ComponentBone*)(*it));
				}	break;
				case ComponentTypes::Skeleton:
				{
					if(InitComponentDraw(*it, "Skeleton"))
						DrawSkeletonComponent((ComponentSkeleton*)(*it));
				}	break;
				case ComponentTypes::Animation:
				{
					if(InitComponentDraw(*it, "Animation"))
						DrawAnimationComponent((ComponentAnimation*)(*it));
				}	break;
				default:
				{
					InitComponentDraw(*it, "Unknown");
				}
			};
		}

	}

    ImGui::End();
}

UID PanelProperties::DrawResource(UID resource, int type)
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
			// Draw All
			UID r = 0;
			r = App->editor->res->DrawResourceType(Resource::texture);
			ret = (r) ? r : ret;
			r = App->editor->res->DrawResourceType(Resource::mesh);
			ret = (r) ? r : ret;
			r = App->editor->res->DrawResourceType(Resource::audio);
			ret = (r) ? r : ret;
			r = App->editor->res->DrawResourceType(Resource::scene);
			ret = (r) ? r : ret;
			r = App->editor->res->DrawResourceType(Resource::bone);
			ret = (r) ? r : ret;
			r = App->editor->res->DrawResourceType(Resource::animation);
			ret = (r) ? r : ret;

		ImGui::EndPopup();
	}
	return ret;
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
	UID new_res = DrawResource(component->GetResourceUID(), Resource::mesh);
	if (new_res > 0)
		component->SetResource(new_res);

	const ResourceMesh* mesh = (const ResourceMesh*) component->GetResource();
	if (mesh == nullptr)
		return;

    ImGui::TextColored(ImVec4(1,1,0,1), "%u Triangles (%u indices %u vertices)",
		mesh->num_indices / 3,
		mesh->num_indices,
		mesh->num_vertices);

	bool uvs = mesh->texture_coords != nullptr;
	bool normals = mesh->normals != nullptr;
	bool colors = mesh->colors != nullptr;

	ImGui::Checkbox("UVs", &uvs);
	ImGui::SameLine();
	ImGui::Checkbox("Normals", &normals);
	ImGui::SameLine();
	ImGui::Checkbox("Colors", &colors);

	ImGui::Text("Potential Bones: %i", component->CountPotentialBones());
	ImGui::Text("Attached to %i bones", component->CountAttachedBones());
	if (component->CountAttachedBones() > 0)
	{
		if (ImGui::Button("DeAttach to all bones"))
			component->DetachBones();
	}
	else
	{
		if (ImGui::Button("Attach to all potential bones"))
			component->AttachBones();
	}
}

void PanelProperties::DrawAudioSourceComponent(ComponentAudioSource * component)
{
	UID new_res = DrawResource(component->GetResourceUID(), Resource::audio);
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
}

void PanelProperties::DrawBoneComponent(ComponentBone * component)
{
	UID new_res = DrawResource(component->GetResourceUID(), Resource::texture);
	if (new_res > 0)
		component->SetResource(new_res);

	if(component->attached_mesh != nullptr)
		ImGui::TextColored(IMGUI_YELLOW, "Attached to %s", component->attached_mesh->GetGameObject()->name.c_str());
	else
		ImGui::TextColored(IMGUI_YELLOW, "Not attached to any mesh");

	ResourceBone* bone = (ResourceBone*) component->GetResource();
	if (bone == nullptr)
		return;

	ImGui::Separator();

	DrawResource(bone->uid_mesh, Resource::mesh);

	ImGui::Text("Num Weigths: %u", bone->num_weigths);

	float3 pos;
	Quat qrot;
	float3 scale;

	bone->offset.Decompose(pos, qrot, scale);

	float3 rot(qrot.ToEulerXYZ());

	bool compose = false;

	if (ImGui::DragFloat3("Offset Trans", (float*)&pos, 0.25f))
		compose = true;

	if(ImGui::SliderFloat3("Offset Rot", (float*)&rot, -PI, PI))
		compose = true;

	if (ImGui::DragFloat3("Offset Scale", (float*)&scale, 0.05f))
		compose = true;

	if (compose == true)
	{
		qrot.FromEulerXYZ(rot.x, rot.y, rot.z);
		bone->offset = float4x4::FromTRS(pos, qrot, scale);
	}
}

void PanelProperties::DrawMaterialComponent(ComponentMaterial * component)
{
	UID new_res = DrawResource(component->GetResourceUID(), Resource::texture);
	if (new_res > 0)
		component->SetResource(new_res);

	const ResourceTexture* info = (const ResourceTexture*) component->GetResource();

	if (info == nullptr)
		return;

	ImGui::Text("(%u,%u) %0.1f Mb", info->width, info->height, info->bytes / (1024.f*1024.f));
	ImGui::Text("Format: %s Depth: %u Bpp: %u Mips: %u", info->GetFormatStr(), info->depth, info->bpp, info->mips);

	ImVec2 size((float)info->width, (float)info->height);
	float max_size = 250.f;

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

	ImGui::Image((ImTextureID) info->gpu_id, size, ImVec2(0,1), ImVec2(1,0), ImColor(255, 255, 255, 128), ImColor(255, 255, 255, 128));
}

void PanelProperties::DrawSkeletonComponent(ComponentSkeleton * component)
{
	ComponentMesh* mesh = component->FindMesh();

	ImGui::Text("Mesh to deform:");
	ImGui::SameLine();
	ImGui::TextColored(IMGUI_YELLOW, "%s", (mesh) ? "OK" : "Please add a mesh component");
}

void PanelProperties::DrawAnimationComponent(ComponentAnimation * component)
{
	UID new_res = DrawResource(component->GetResourceUID(), Resource::texture);
	if (new_res > 0)
		component->SetResource(new_res);

	const ResourceAnimation* info = (const ResourceAnimation*) component->GetResource();

	if (info == nullptr)
		return;

	ImGui::Text("Name: %s", info->name.c_str());
	ImGui::Text("Duration: %f", info->duration);
	ImGui::Text("Ticks Per Second: %f", info->duration);
}
