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
