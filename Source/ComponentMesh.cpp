#include "Globals.h"
#include "ComponentMesh.h"
#include "ModuleMeshes.h"

// ---------------------------------------------------------
ComponentMesh::ComponentMesh(GameObject* container) : Component(container)
{
	type = ComponentTypes::Geometry;
}

// ---------------------------------------------------------
void ComponentMesh::SetMesh(const Mesh * data)
{
	if (data != nullptr)
	{
		mesh_data = data;
		bounding_box.SetFrom((float3*) mesh_data->vertices, data->num_indices / 3);
	}
}

const Mesh * ComponentMesh::GetMesh() const
{
	return mesh_data;
}

const AABB* ComponentMesh::GetBoundingBox() const
{
	return &bounding_box;
}
