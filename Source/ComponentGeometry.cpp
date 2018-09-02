#include "Globals.h"

#include "ComponentGeometry.h"


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
        meshes.push_back(mesh_indices[i]);
    }
}

void ComponentGeometry::OnSave(Config& config) const 
{
    config.AddArrayUID("Meshes", &meshes[0], meshes.size());
}

void ComponentGeometry::OnLoad(Config* config) 
{
    meshes.clear();
   
	config->GetUID("Meshes", 0);
}

