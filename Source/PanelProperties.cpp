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
#include "ComponentRigidBody.h"
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
#include "PanelGOTree.h"
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
			if (ImGui::MenuItem("RigidBody"))
				selected->CreateComponent(ComponentTypes::RigidBody);
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
			if (InitComponentDraw(*it, (*it)->GetTypeStr()))
			{
				ImGui::PushID(*it);

				switch ((*it)->GetType())
				{
				case ComponentTypes::Geometry:
				{
					DrawMeshComponent((ComponentMesh*)(*it));
				}	break;
				case ComponentTypes::Material:
				{
					DrawMaterialComponent((ComponentMaterial*)(*it));
				}	break;
				case ComponentTypes::AudioSource:
				{
					DrawAudioSourceComponent((ComponentAudioSource*)(*it));
				}	break;
				case ComponentTypes::AudioListener:
				{
					DrawAudioListenerComponent((ComponentAudioListener*)(*it));
				}	break;
				case ComponentTypes::Camera:
				{
					DrawCameraComponent((ComponentCamera*)(*it));
				}	break;
				case ComponentTypes::Bone:
				{
					DrawBoneComponent((ComponentBone*)(*it));
				}	break;
				case ComponentTypes::RigidBody:
				{
					DrawRigidBodyComponent((ComponentRigidBody*)(*it));
				}	break;
				case ComponentTypes::Animation:
				{
					DrawAnimationComponent((ComponentAnimation*)(*it));
				}	break;
				}
				ImGui::PopID();
			}
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
	UID new_res = PickResource(component->GetResourceUID(), Resource::mesh);
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

	const GameObject* selected = PickGameObject(component->root_bones);
	if (selected != nullptr)
		component->AttachBones(selected);

	if (component->root_bones != nullptr)
	{
		ImGui::SameLine();
		if (ImGui::Button("Dettach"))
			component->DetachBones();
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
}

void PanelProperties::DrawBoneComponent(ComponentBone * component)
{
	UID new_res = PickResource(component->GetResourceUID(), Resource::texture);
	if (new_res > 0)
		component->SetResource(new_res);

	ImGui::Checkbox("Translation Locked", &component->translation_locked);

	if(component->attached_mesh != nullptr)
		ImGui::TextColored(IMGUI_YELLOW, "Attached to %s", component->attached_mesh->GetGameObject()->name.c_str());
	else
		ImGui::TextColored(IMGUI_YELLOW, "Not attached to any mesh");

	ResourceBone* bone = (ResourceBone*) component->GetResource();
	if (bone == nullptr)
		return;

	ImGui::Separator();

	PickResource(bone->uid_mesh, Resource::mesh);

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
	UID new_res = PickResource(component->GetResourceUID(), Resource::texture);
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

void PanelProperties::DrawAnimationComponent(ComponentAnimation * component)
{
	UID new_res = PickResource(component->current->resource, Resource::animation);
	if (new_res > 0)
		component->SetResource(new_res);

	const ResourceAnimation* info = (const ResourceAnimation*) component->GetResource();

	if (info == nullptr)
		return;

	ImGui::Text("Name: %s", info->name.c_str());
	ImGui::Text("Duration in Ticks: %.3f", info->duration);
	ImGui::Text("Ticks Per Second: %.3f", info->ticks_per_second);
	ImGui::Text("Real Time: %.3f", info->GetDurationInSecs());
	ImGui::Text("Potential Bones to animate: %i", component->current->CountBones());

	static const char * states[] = { 
		"Not Loaded", 
		"Stopped",
		"About to Play",
		"Playing",
		"About to Pause",
		"Pause",
		"About to Unpause",
		"About to Stop",
		"Blending",
		"About to Blend"
	};

	ImGui::Text("Current State: ");
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", states[component->GetCurrentState()]);
	ImGui::Text("Attached to Bones: %i", component->current->CountAttachedBones());
	ImGui::Checkbox("Interpolate", &component->interpolate);
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &component->current->loop);
	ImGui::SliderFloat("Speed", &component->current->speed, -5.f, 5.f);
	ImGui::Text("Animation Time: %.3f", component->current->GetTime());
	ImGui::ProgressBar(component->current->GetTime() / info->GetDurationInSecs());

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

	ImGui::Separator();
	ImGui::Text("Blending to another animation");

	static float blending_time_requested = 5.0f;
	ImGui::SliderFloat("Time to blend", &blending_time_requested, 0.0f, 10.0f);

	ImGui::PushID(2);
	UID new_res2 = PickResource(component->next->resource, Resource::animation);
	if (new_res2 > 0)
		component->BlendTo(new_res2, blending_time_requested);
	ImGui::PopID();

	const ResourceAnimation* info2 = component->next->GetResource();

	ImGui::ProgressBar(component->blend_time / component->total_blend_time);
	ImGui::ProgressBar(1.0f - (component->blend_time / component->total_blend_time));

	ImGui::Separator();

	if (ImGui::TreeNode("Bone Transformations"))
	{
		for (uint i = 0; i < info->num_keys; ++i)
		{
			if (ImGui::TreeNode(info->bone_keys[i].bone_name.c_str()))
			{
				ResourceAnimation::bone_transform* bone = &info->bone_keys[i];

				// Positions ---
				if (ImGui::TreeNode("Positions", "Positions (%i)", bone->positions.count))
				{
					ImGui::Columns(2, "Position");
					for (uint k = 0; k < bone->positions.count; ++k)
					{
						ImGui::Text("%.1f", info->bone_keys[i].positions.time[k]);
						ImGui::NextColumn();
						ImGui::Text("%.1f, %.1f, %.1f",
							info->bone_keys[i].positions.value[k * 3 + 0],
							info->bone_keys[i].positions.value[k * 3 + 1],
							info->bone_keys[i].positions.value[k * 3 + 2]);
						ImGui::NextColumn();
					}
					ImGui::Columns(1);
					ImGui::TreePop();
				}

				// Rotations ---
				if (ImGui::TreeNode("Rotations", "Rotations (%i)", bone->rotations.count))
				{
					ImGui::Columns(2, "Rotation");
					for (uint k = 0; k < bone->rotations.count; ++k)
					{
						ImGui::Text("%.1f", info->bone_keys[i].rotations.time[k]);
						ImGui::NextColumn();
						ImGui::Text("%.1f,%.1f,%.1f,%.1f",
							info->bone_keys[i].rotations.value[k * 4 + 0],
							info->bone_keys[i].rotations.value[k * 4 + 1],
							info->bone_keys[i].rotations.value[k * 4 + 2],
							info->bone_keys[i].rotations.value[k * 4 + 3]);
						ImGui::NextColumn();
					}
					ImGui::Columns(1);
					ImGui::TreePop();
				}

				// Scales ---
				if (ImGui::TreeNode("Scales", "Scales (%i)", bone->scales.count))
				{
					ImGui::Columns(2, "Scale");
					for (uint k = 0; k < bone->scales.count; ++k)
					{
						ImGui::Text("%.1f", info->bone_keys[i].scales.time[k]);
						ImGui::NextColumn();
						ImGui::Text("%.1f, %.1f, %.1f",
							info->bone_keys[i].scales.value[k * 3 + 0],
							info->bone_keys[i].scales.value[k * 3 + 1],
							info->bone_keys[i].scales.value[k * 3 + 2]);
						ImGui::NextColumn();
					}
					ImGui::Columns(1);
					ImGui::TreePop();
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}

void PanelProperties::DrawRigidBodyComponent(ComponentRigidBody * component)
{
	component->DrawEditor();
}
