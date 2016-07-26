#ifndef __MODULE_MESHES_H__
#define __MODULE_MESHES_H__

#include "Globals.h"
#include "Module.h"
#include <list>

#define INVALID_MESH 0

// full mesh description
struct Mesh
{
	uint id = INVALID_MESH;
	// ids of the VBO
	uint vbo_vertices = 0;
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

	~Mesh()
	{
		RELEASE_ARRAY(indices);
		RELEASE_ARRAY(vertices);
		RELEASE_ARRAY(colors);
		RELEASE_ARRAY(normals);
		RELEASE_ARRAY(texture_coords);
		// TODO: deallocate opengl buffers
	}
};

// Mesh manager, keeps in memory all meshes only once
struct aiMesh;

class ModuleMeshes : public Module
{
public:
	ModuleMeshes(bool start_enabled = true);
	~ModuleMeshes();

	bool Init(Config* config = nullptr);
	bool CleanUp();

	const Mesh* Load(const aiMesh* mesh);
	uint GenerateVertexBuffer(const Mesh* mesh);

private:
	std::list<Mesh*> meshes;
};

#endif // __MODULE_MESHES_H__