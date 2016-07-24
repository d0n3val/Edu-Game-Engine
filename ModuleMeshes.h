#ifndef __MODULE_MESHES_H__
#define __MODULE_MESHES_H__

#include "Globals.h"
#include "Module.h"
#include <list>

#define INVALID_MESH 0

// Single face of the mesh (poly)
struct face
{
	uint num_indices = 0;
	uint* indices = nullptr;

	~face()
	{
		RELEASE_ARRAY(indices);
	}
};

// full mesh description
struct mesh
{
	uint id = INVALID_MESH;
	uint num_faces = 0;
	face* faces = nullptr;
	uint num_vertices = 0;
	float* vertices = nullptr;
	float* colors = nullptr;
	float* normals = nullptr;
	float* texture_coords = nullptr;

	~mesh()
	{
		RELEASE_ARRAY(faces);
		RELEASE_ARRAY(vertices);
		RELEASE_ARRAY(colors);
		RELEASE_ARRAY(normals);
		RELEASE_ARRAY(texture_coords);
	}
};

// Mesh manager, keeps in memory all meshes only once
struct aiMesh;

class ModuleMeshes : public Module
{
public:
	ModuleMeshes(bool start_enabled = true);
	~ModuleMeshes();

	bool Init();
	bool CleanUp();

	uint Load(const aiMesh* mesh);

private:
	std::list<mesh*> meshes;
};

#endif // __MODULE_MESHES_H__