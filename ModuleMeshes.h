#ifndef __MODULE_MESHES_H__
#define __MODULE_MESHES_H__

#include "Globals.h"
#include "Module.h"
#include <list>

#define INVALID_MESH 0

// Single face of the mesh (poly)
struct Face
{
	uint num_indices = 0;
	uint* indices = nullptr;

	~Face()
	{
		RELEASE_ARRAY(indices);
	}
};

// full mesh description
struct Mesh
{
	uint id = INVALID_MESH;
	uint num_faces = 0;
	Face* faces = nullptr;
	uint num_vertices = 0;
	float* vertices = nullptr;
	float* colors = nullptr;
	float* normals = nullptr;
	float* texture_coords = nullptr;

	~Mesh()
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

	const Mesh* Load(const aiMesh* mesh);

private:
	std::list<Mesh*> meshes;
};

#endif // __MODULE_MESHES_H__