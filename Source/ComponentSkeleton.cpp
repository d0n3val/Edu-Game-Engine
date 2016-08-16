#include "Globals.h"
#include "Application.h"
#include "ComponentSkeleton.h"
#include "ModuleMeshes.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"
#include "GameObject.h"
#include "Component.h"
#include "ComponentMesh.h"
#include <list>

using namespace std;

// ---------------------------------------------------------
ComponentSkeleton::ComponentSkeleton(GameObject* container) : Component(container)
{
	type = ComponentTypes::Skeleton;
}

// ---------------------------------------------------------
void ComponentSkeleton::OnSave(Config& config) const
{
}

// ---------------------------------------------------------
void ComponentSkeleton::OnLoad(Config * config)
{
}

// ---------------------------------------------------------
ComponentMesh * ComponentSkeleton::FindMesh() const
{
	// look for a mesh to deform: since gameobjects can only have one mesh
	// we just take the first componentmesh we find in this game object

	ComponentMesh* ret = nullptr;

	for (list<Component*>::iterator it = game_object->components.begin(); it != game_object->components.end(); ++it)
	{
		if ((*it)->GetType() == ComponentTypes::Geometry)
			ret = (ComponentMesh*)*it;
	}

	return ret;
}
