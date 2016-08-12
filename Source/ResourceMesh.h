#ifndef __RESOURCE_MESH_H__
#define __RESOURCE_MESH_H__

#include "Resource.h"

class ResourceMesh : public Resource
{
	friend class ModuleMeshes;

public:
	ResourceMesh(UID id);
	virtual ~ResourceMesh();

public:
	uint vbo_vertices = 0;
	uint vbo_colors = 0;
	uint vbo_normals = 0;
	uint vbo_texture_coords = 0;
	uint vbo_indices = 0;

	uint num_indices = 0;
	uint num_vertices = 0;
	uint* indices = nullptr;
	float* vertices = nullptr;
	float* colors = nullptr;
	float* normals = nullptr;
	float* texture_coords = nullptr;
};

#endif // __RESOURCE_MESH_H__