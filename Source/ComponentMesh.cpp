#include "Globals.h"
#include "Application.h"
#include "ComponentMesh.h"
#include "ModuleMeshes.h"
#include "ModuleResources.h"
#include "ModuleLevelManager.h"
#include "ResourceMesh.h"
#include "GameObject.h"
#include "ComponentBone.h"
#include "ResourceBone.h"
#include <vector>

using namespace std;

// ---------------------------------------------------------
ComponentMesh::ComponentMesh(GameObject* container) : Component(container, Types::Geometry)
{}

// ---------------------------------------------------------
ComponentMesh::~ComponentMesh()
{
	RELEASE(deformable);
}

// ---------------------------------------------------------
void ComponentMesh::OnSave(Config& config) const
{
	ComponentWithResource::OnSaveResource(config);
	// TODO naive approach to bone attachment, simply try on start
	config.AddInt("Bones Root", (root_bones) ? root_bones->serialization_id : 0);
}

// ---------------------------------------------------------
void ComponentMesh::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
	root_bones_id = config->GetInt("Bones Root", 0);
}

// ---------------------------------------------------------
bool ComponentMesh::SetResource(UID resource)
{
	bool ret = false;

	if (resource != 0)
	{
		Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->GetType() == Resource::mesh)
		{
			if(res->LoadToMemory() == true)
			{
				this->resource = resource;
				ret = true;
			}
		}
	}

	return true;
}

// ---------------------------------------------------------
void ComponentMesh::OnStart()
{
	const GameObject* go = App->level->Find(root_bones_id, App->level->GetRoot());

	if (go != nullptr)
		AttachBones(go);
}

// ---------------------------------------------------------
const AABB & ComponentMesh::GetBoundingBox() const
{
	const ResourceMesh* res = (const ResourceMesh*) App->resources->Get(resource);
	if (res != nullptr)
		return res->bbox;
	static AABB inf;
	inf.SetNegativeInfinity();
	return inf;
}

// ---------------------------------------------------------
uint ComponentMesh::CountPotentialBones() const
{
	vector<ComponentBone*> bones;
	if(root_bones != nullptr)
		RecursiveFindBones(root_bones, bones);

	return bones.size();
}

// ---------------------------------------------------------
void ComponentMesh::AttachBones(const GameObject* go)
{
	vector<ComponentBone*> bones;
	RecursiveFindBones(go, bones);

	if (bones.size() > 0)
	{
		DetachBones();
		root_bones = go;
		attached_bones = bones;

		if (deformable == nullptr)
		{
			deformable = new ResourceMesh(0);
			deformable->CreateDeformableVersion((const ResourceMesh*)GetResource());
			App->meshes->GenerateVertexBuffer(deformable);
		}

		for (vector<ComponentBone*>::iterator it = attached_bones.begin(); it != attached_bones.end(); ++it)
			(*it)->attached_mesh = this;
	}
}

// ---------------------------------------------------------
void ComponentMesh::DetachBones()
{
	for (vector<ComponentBone*>::iterator it = attached_bones.begin(); it != attached_bones.end(); ++it)
		(*it)->attached_mesh = nullptr;
	attached_bones.clear();
	RELEASE(deformable);
}

// ---------------------------------------------------------
uint ComponentMesh::CountAttachedBones() const
{
	return attached_bones.size();
}

// ---------------------------------------------------------
void ComponentMesh::ResetDeformableMesh()
{
	if (deformable != nullptr)
	{
		const ResourceMesh* original = (const ResourceMesh*) GetResource();

		memset(deformable->indices, 0, original->num_indices * sizeof(uint));

		memcpy(deformable->vertices, original->vertices, deformable->num_vertices * sizeof(float) * 3);
		//memset(deformable->vertices, 0, deformable->num_vertices * sizeof(float) * 3);

		if (deformable->normals != nullptr)
			memcpy(deformable->normals, original->normals, deformable->num_vertices * sizeof(float) * 3);
			//memset(deformable->normals, original->normals, deformable->num_vertices * sizeof(float) * 3);
	}
}

// ---------------------------------------------------------
void ComponentMesh::RecursiveFindBones(const GameObject * go, vector<ComponentBone*>& found) const
{
	if (go == nullptr)
		return;

	for (list<Component*>::const_iterator it = go->components.begin(); it != go->components.end(); ++it)
	{
		if ((*it)->GetType() == Component::Types::Bone)
		{
			ComponentBone* bone = (ComponentBone*)*it;
			ResourceBone* res = (ResourceBone*) bone->GetResource();

			if (res != nullptr && res->uid_mesh == GetResourceUID())
			{
				found.push_back(bone);
			}
		}
	}

	for (list<GameObject*>::const_iterator it = go->childs.begin(); it != go->childs.end(); ++it)
		RecursiveFindBones(*it, found);
}
