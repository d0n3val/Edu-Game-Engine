#include "Globals.h"

#include "ComponentGeometry.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ResourceMesh.h"

ComponentGeometry::ComponentGeometry(GameObject* go) : Component(go, Types::Geometry)
{
}

ComponentGeometry::~ComponentGeometry()
{
    for(uint i=0; i< meshes.size(); ++i)
    {
        App->resources->Get(meshes[i])->Release();
    }
}

void ComponentGeometry::Initialize(const UID* ids, const unsigned* mesh_indices, unsigned count)
{
    meshes.reserve(count);

    for(unsigned i=0; i < count; ++i)
    {
		assert(ids[mesh_indices[i]] != 0);
        meshes.push_back(ids[mesh_indices[i]]);

        App->resources->Get(meshes.back())->LoadToMemory();
    }
}

void ComponentGeometry::OnSave(Config& config) const 
{
    config.AddArrayUID("Meshes", !meshes.empty() ? &meshes[0] : nullptr, meshes.size());
}

void ComponentGeometry::OnLoad(Config* config) 
{
	uint num_meshes = config->GetArrayCount("Meshes");
    meshes.reserve(num_meshes);

    for(uint i=0; i< num_meshes; ++i)
    {
        meshes.push_back(config->GetUID("Meshes", 0, i));
    }

    for(uint i=0; i< num_meshes; ++i)
    {
        App->resources->Get(meshes[i])->LoadToMemory();
    }
}

void ComponentGeometry::GetBoundingBox (AABB& box) const 
{
    for(uint i=0; i< meshes.size(); ++i)
    {
        box.Enclose(static_cast<ResourceMesh*>(App->resources->Get(meshes[i]))->bbox);
    }
}


