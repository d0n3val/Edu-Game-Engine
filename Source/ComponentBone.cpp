#include "ComponentBone.h"
#include "Globals.h"
#include "Application.h"
#include "ModuleResources.h"
#include "GameObject.h"
#include "Math.h"
#include "DebugDraw.h"
#include "Color.h"
#include "Resource.h"
#include "ResourceBone.h"
#include "ResourceMesh.h"
#include "ComponentMesh.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
ComponentBone::ComponentBone(GameObject* container) : Component(container, Types::Bone)
{}

// ---------------------------------------------------------
ComponentBone::~ComponentBone()
{
}

// ---------------------------------------------------------
void ComponentBone::OnSave(Config & config) const
{
	ComponentWithResource::OnSaveResource(config);
	if(translation_locked == true)
	config.AddBool("Pos Locked", translation_locked);
}

// ---------------------------------------------------------
void ComponentBone::OnLoad(Config * config)
{
	ComponentWithResource::OnLoadResource(config);
	translation_locked = config->GetBool("Pos Locked", false);
}

// ---------------------------------------------------------
bool ComponentBone::SetResource(UID resource)
{
	bool ret = false;

	if (resource != 0)
	{
		Resource* res = App->resources->Get(resource);
		if (res != nullptr && res->GetType() == Resource::bone)
		{
			if(res->LoadToMemory() == true)
			{
				this->resource = resource;
				ret = true;
			}
		}
	}

	return ret;
}

// ---------------------------------------------------------
void ComponentBone::OnDebugDraw(bool selected) const
{
	if (selected == false)
		return;

	math::LineSegment segment;
	segment.a = game_object->GetGlobalPosition();

	for (list<GameObject*>::const_iterator it = game_object->childs.begin(); it != game_object->childs.end(); ++it)
	{
		segment.b = (*it)->GetGlobalPosition();
		DebugDraw(segment, Blue);
	}

	if (attached_mesh != nullptr)
	{
		const ResourceBone* bone = (const ResourceBone*) GetResource();
		const ResourceMesh* mesh = (const ResourceMesh*) attached_mesh->deformable;

		if (mesh != nullptr && bone != nullptr)
		{
			for (uint i = 0; i < bone->num_weigths; ++i)
			{
				float3 vertex(mesh->src_vertices[bone->weigth_indices[i] * 3]);
				vertex = attached_mesh->GetGameObject()->GetGlobalTransformation().TransformPos(vertex);
				Color c(0.f, 1.f - bone->weigths[i], bone->weigths[i]);
				DebugDraw(vertex, c);
			}
		}
	}
}
