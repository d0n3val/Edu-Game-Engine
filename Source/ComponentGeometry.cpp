#include "Globals.h"

#include "ComponentGeometry.h"


ComponentGeometry::ComponentGeometry()
{
}

ComponentGeometry::~ComponentGeometry()
{
}

void ComponentGeometry::Initialize(const std::vector<UID>& m)
{
    meshes = m;
}

void ComponentGeometry::OnSave(Config& config) const 
{
    config.AddArrayUInt("Meshes", &meshes[0], meshes.size());
}

void ComponentGeometry::OnLoad(Config* config) 
{
    meshes.clear();
   
	config->GetUID("Meshes", 0);
}

