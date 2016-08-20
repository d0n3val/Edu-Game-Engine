#include "Globals.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleAnimation.h"
#include "GameObject.h"
#include "ComponentBone.h"
#include "ResourceBone.h"
#include "ComponentMesh.h"
#include "ComponentAnimation.h"
#include "ResourceMesh.h"
#include "ResourceAnimation.h"
#include "DebugDraw.h"

using namespace std;

ModuleAnimation::ModuleAnimation(bool start_active) : Module("Animation", start_active)
{
}

// Destructor
ModuleAnimation::~ModuleAnimation()
{
}

// Called before render is available
bool ModuleAnimation::Init(Config* config)
{
	LOG("Loading Animation System");
	bool ret = true;

	return ret;
}

// Called before quitting
bool ModuleAnimation::CleanUp()
{
	LOG("Freeing Animation subsystem");

	return true;
}

// ---------------------------------------------------------
update_status ModuleAnimation::Update(float dt)
{
	// Update all bones according to animations
	RecursiveUpdateAnimation(App->level->GetRoot(), dt);

	// Reset all deformable meshes, getting them ready to receive transformation
	RecursiveResetMeshes(App->level->GetRoot());

	// Deform meshes attached to bones
	RecursiveDeformMeshes(App->level->GetRoot());

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
void ModuleAnimation::RecursiveUpdateAnimation(GameObject * go, float dt)
{
	for (list<GameObject*>::iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		if((*it)->IsActive())
			RecursiveUpdateAnimation(*it, dt);

	for (list<Component*>::iterator it = go->components.begin(); it != go->components.end(); ++it)
		if ((*it)->IsActive() && (*it)->GetType() == ComponentTypes::Animation)
			UpdateAnimation((ComponentAnimation*)*it, dt);
}

// ---------------------------------------------------------
void ModuleAnimation::RecursiveResetMeshes(GameObject * go)
{
	for (list<GameObject*>::iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		if((*it)->IsActive())
			RecursiveResetMeshes(*it);

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
		if ((*it)->IsActive() && (*it)->GetType() == ComponentTypes::Geometry)
			((ComponentMesh*)*it)->ResetDeformableMesh();
}
// ---------------------------------------------------------
void ModuleAnimation::RecursiveDeformMeshes(GameObject * go)
{
	for (list<GameObject*>::iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		if((*it)->IsActive())
			RecursiveDeformMeshes(*it);

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
		if ((*it)->IsActive() && (*it)->GetType() == ComponentTypes::Bone)
			DeformMesh((const ComponentBone*)*it);
}

// ---------------------------------------------------------
void ModuleAnimation::UpdateAnimation(ComponentAnimation * anim, float dt)
{
	const ResourceAnimation* res = (const ResourceAnimation*) anim->GetResource();
	if (res != nullptr && anim->current_state)
	{
		const GameObject* go = anim->GetGameObject();

		switch (anim->GetCurrentState())
		{
		case ComponentAnimation::state::waiting_to_play:
			anim->current->AttachBones();
			anim->current->time = 0.f;
			if(anim->current->bones.size() > 0)
				anim->current_state = ComponentAnimation::state::playing;
		break;
		case ComponentAnimation::state::waiting_to_stop:
			anim->current_state = ComponentAnimation::state::stopped;
		break;
		case ComponentAnimation::state::waiting_to_blend:
			anim->next->AttachBones();
			if(anim->next->bones.size() > 0)
				anim->current_state = ComponentAnimation::state::blending;
		//break; we want to fall down here!
		case ComponentAnimation::state::playing:
		{
			if (AdvanceAnimation(anim->current, dt) == false)
				anim->current_state = ComponentAnimation::state::stopped;
		}	break;
		case ComponentAnimation::state::blending:
		{
			anim->blend_time += dt;
			float blend_factor = anim->blend_time / anim->total_blend_time;

			AdvanceAnimation(anim->current, dt);
			AdvanceAnimation(anim->next, dt, blend_factor);

			// Finish blend process
			if (anim->blend_time > anim->total_blend_time)
			{
				anim->SwitchChannels();
				anim->current_state = ComponentAnimation::state::playing;
			}
		}	break;
		default:
			break;
		}
	}
}

// ---------------------------------------------------------
bool ModuleAnimation::AdvanceAnimation(ComponentAnimation::Channel * anim, float dt, float blend)
{
	const ResourceAnimation* res = (const ResourceAnimation*) anim->GetResource();

	// advance animation timer
	anim->time += dt * anim->speed;

	// are we done ? (taking in account negative speeds)
	if (anim->time > res->GetDurationInSecs())
	{
		if (anim->loop == true)
			anim->time = anim->time - res->GetDurationInSecs();
		else
			return false;
	}
	else if (anim->time < 0 && anim->speed < 0.0f)
	{
		if (anim->loop == true)
			anim->time = res->GetDurationInSecs() - anim->time;
		else
			return false;
	}

	float3 pos;
	Quat rot;
	float3 scale;

	for (map<uint, ComponentBone*>::iterator it = anim->bones.begin(); it != anim->bones.end(); ++it)
	{
		res->FindBoneTransformation(anim->time, it->first, pos, rot, scale, anim->component->interpolate);

		GameObject* go = it->second->GetGameObject();

		if (blend >= 1.f)
		{
			if(it->second->translation_locked == false)
				go->SetLocalPosition(pos);
			go->SetLocalRotation(rot);
			go->SetLocalScale(scale);
		}
		else
		{
			if(it->second->translation_locked == false) 
				go->SetLocalPosition(float3::Lerp(go->GetLocalPosition(), pos, blend));
			go->SetLocalRotation(Quat::Slerp(go->GetLocalRotationQ(), rot, blend));
			go->SetLocalScale(float3::Lerp(go->GetLocalScale(), scale, blend));
		}
	}

	return true;
}

// ---------------------------------------------------------
void ModuleAnimation::DeformMesh(const ComponentBone* bone)
{
	ComponentMesh* mesh = bone->attached_mesh;

	if(mesh != nullptr)
	{
		const ResourceBone* rbone = (const ResourceBone*) bone->GetResource();
		const ResourceMesh* roriginal = (const ResourceMesh*) mesh->GetResource();
		ResourceMesh* rmesh = (ResourceMesh*) mesh->deformable;

		// calc the transformation of this bone based on its root (not the global transformation)
		float4x4 trans = bone->GetGameObject()->GetGlobalTransformation();
		trans = trans * bone->attached_mesh->GetGameObject()->GetLocalTransform().Inverted();

		// Now apply a transformation to place the vertex as it was in the bind pose
		trans = trans * rbone->offset;

		for (uint i = 0; i < rbone->num_weigths; ++i)
		{
			uint index = rbone->weigth_indices[i];
			float3 original(&roriginal->vertices[index * 3]);
			float3 vertex(&rmesh->vertices[index*3]);

			if (rmesh->indices[index]++ == 0) 
			{
				memset(&rmesh->vertices[index*3], 0, sizeof(float) * 3);
				if(roriginal->normals)
					memset(&rmesh->normals[index*3], 0, sizeof(float) * 3);
			}
			
			vertex = trans.TransformPos(original);

			rmesh->vertices[index * 3] += vertex.x * rbone->weigths[i];
			rmesh->vertices[index * 3 + 1] += vertex.y * rbone->weigths[i];
			rmesh->vertices[index * 3 + 2] += vertex.z * rbone->weigths[i];

			if (roriginal->normals)
			{
				vertex = trans.TransformPos(float3(&roriginal->normals[index*3]));
				rmesh->normals[index * 3] += vertex.x * rbone->weigths[i];
				rmesh->normals[index * 3 + 1] += vertex.y * rbone->weigths[i];
				rmesh->normals[index * 3 + 2] += vertex.z * rbone->weigths[i];
			}
		}
	}
}
