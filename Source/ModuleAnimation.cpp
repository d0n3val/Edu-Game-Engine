#include "Globals.h"
#include "Application.h"
#include "ModuleLevelManager.h"
#include "ModuleAnimation.h"
#include "GameObject.h"
#include "ComponentBone.h"
#include "ResourceBone.h"
#include "ComponentMesh.h"
#include "ResourceMesh.h"
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
	// Reset all deformable meshes, getting them ready to receive transformation
	RecursiveResetMeshes(App->level->GetRoot());

	// Deform meshes attached to bones
	RecursiveDeformMeshes(App->level->GetRoot());

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
void ModuleAnimation::RecursiveResetMeshes(GameObject * go)
{
	for (list<GameObject*>::iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		if((*it)->IsActive())
			RecursiveResetMeshes(*it);

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->IsActive() && (*it)->GetType() == ComponentTypes::Geometry)
			((ComponentMesh*)*it)->ResetDeformableMesh();
	}

}
// ---------------------------------------------------------
void ModuleAnimation::RecursiveDeformMeshes(GameObject * go)
{
	for (list<GameObject*>::iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		if((*it)->IsActive())
			RecursiveDeformMeshes(*it);

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->IsActive() && (*it)->GetType() == ComponentTypes::Bone)
			DeformMesh((const ComponentBone*)*it);
	}
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
