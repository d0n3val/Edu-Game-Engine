#include "Globals.h"

#include "ComponentGeometry.h"
#include "Application.h"
#include "ModuleResources.h"


ComponentGeometry::ComponentGeometry(GameObject* go) : Component(go, Types::Geometry)
{
}

ComponentGeometry::~ComponentGeometry()
{
}

void ComponentGeometry::Initialize(const UID* ids, const unsigned* mesh_indices, unsigned count)
{
    meshes.clear();
    meshes.reserve(count);

    for(unsigned i=0; i < count; ++i)
    {
		assert(ids[mesh_indices[i]] != 0);
        meshes.push_back(ids[mesh_indices[i]]);

        App->resources->Get(meshes.back())->LoadToMemory();
        // \todo: Unload!!!!
    }
}

void ComponentGeometry::OnSave(Config& config) const 
{
    config.AddArrayUID("Meshes", !meshes.empty() ? &meshes[0] : nullptr, meshes.size());
}

void ComponentGeometry::OnLoad(Config* config) 
{
    meshes.clear();
   
	config->GetUID("Meshes", 0);
}

