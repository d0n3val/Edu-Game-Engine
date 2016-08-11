#include "Globals.h"
#include "ComponentMesh.h"
#include "ModuleMeshes.h"
#include "GameObject.h"

// ---------------------------------------------------------
ComponentMesh::ComponentMesh(GameObject* container) : Component(container)
{
	type = ComponentTypes::Geometry;
	bbox.SetNegativeInfinity();
}

// ---------------------------------------------------------
void ComponentMesh::OnSave(Config& config) const
{
	config.AddArrayFloat("AABB", (float*) &bbox.minPoint.x, 6);
}

// ---------------------------------------------------------
void ComponentMesh::OnLoad(Config * config)
{
	bbox.minPoint.x = config->GetFloat("AABB", 0.f, 0);
	bbox.minPoint.y = config->GetFloat("AABB", 0.f, 1);
	bbox.minPoint.z = config->GetFloat("AABB", 0.f, 2);

	bbox.maxPoint.x = config->GetFloat("AABB", 0.f, 3);
	bbox.maxPoint.y = config->GetFloat("AABB", 0.f, 4);
	bbox.maxPoint.z = config->GetFloat("AABB", 0.f, 5);
}

// ---------------------------------------------------------
void ComponentMesh::SetMesh(const Mesh * data)
{
	if (data != nullptr)
	{
		mesh_data = data;
		bbox.Enclose((float3*)mesh_data->vertices, data->num_vertices);
	}
}

// ---------------------------------------------------------
const Mesh * ComponentMesh::GetMesh() const
{
	return mesh_data;
}

const AABB & ComponentMesh::GetBoundingBox() const
{
	return bbox;
}
