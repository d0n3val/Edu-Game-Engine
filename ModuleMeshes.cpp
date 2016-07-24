#include "Globals.h"
#include "Application.h"
#include "ModuleMeshes.h"
#include "Assimp/include/mesh.h"

using namespace std;

ModuleMeshes::ModuleMeshes(bool start_enabled) : Module(start_enabled)
{
}

// Destructor
ModuleMeshes::~ModuleMeshes()
{
}

// Called before render is available
bool ModuleMeshes::Init()
{
	LOG("Init Mesh library");

	return true;
}

// Called before quitting
bool ModuleMeshes::CleanUp()
{
	LOG("Freeing Mesh library");
					  
	for(list<mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it)
		RELEASE(*it);

	meshes.clear();
	return true;
}

// Load new texture from file path
const mesh* ModuleMeshes::Load(const aiMesh* new_mesh)
{
	if (new_mesh == nullptr)
		return INVALID_MESH;

	mesh* m = new mesh;

	// copy vertices
	m->num_vertices = new_mesh->mNumVertices;
	m->vertices = new float[m->num_vertices * 3];
	memcpy(m->vertices, new_mesh->mVertices, m->num_vertices * 3);

	// copy faces
	if (new_mesh->HasFaces())
	{
		m->num_faces = new_mesh->mNumFaces;
		m->faces = new face[m->num_faces];
		for (uint i = 0; i < m->num_faces; ++i)
		{
			uint num_faces = new_mesh->mFaces[i].mNumIndices; 

			m->faces[i].num_indices = num_faces;
			m->faces[i].indices = new uint[num_faces];
			memcpy(m->faces[i].indices, new_mesh->mFaces[i].mIndices, num_faces);
		}
	}

	// normals
	if (new_mesh->HasNormals())
	{
		m->normals = new float[m->num_vertices * 3];
		memcpy(m->normals, new_mesh->mNormals, m->num_vertices * 3);
	}

	// texture coords (only one texture for now)
	if (new_mesh->HasTextureCoords(0))
	{
		m->texture_coords = new float[m->num_vertices * 2];
		memcpy(m->texture_coords, new_mesh->mTextureCoords, m->num_vertices * 3);
	}

	meshes.push_back(m);
	return m;
}