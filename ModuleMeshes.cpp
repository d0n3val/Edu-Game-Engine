#include "Globals.h"
#include "Application.h"
#include "ModuleMeshes.h"
#include "Glew/include/glew.h" // extensio lib
#include "gl/GL.h"
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
					  
	for(list<Mesh*>::iterator it = meshes.begin(); it != meshes.end(); ++it)
		RELEASE(*it);

	meshes.clear();
	return true;
}

// Load new texture from file path
const Mesh* ModuleMeshes::Load(const aiMesh* new_mesh)
{
	if (new_mesh == nullptr)
		return INVALID_MESH;

	Mesh* m = new Mesh;

	// copy vertices
	m->num_vertices = new_mesh->mNumVertices;
	m->vertices = new float[m->num_vertices * 3];
	memcpy(m->vertices, new_mesh->mVertices, sizeof(float) * m->num_vertices * 3);

/*	
	GLuint vbo = 0;
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, sizeof(float) * m->num_vertices * 3, m->vertices, GL_STATIC_DRAW);
	*/  
	// copy faces
	if (new_mesh->HasFaces())
	{
		m->num_faces = new_mesh->mNumFaces;
		m->faces = new Face[m->num_faces];
		for (uint i = 0; i < m->num_faces; ++i)
		{
			uint num_indices = new_mesh->mFaces[i].mNumIndices; 

			m->faces[i].num_indices = num_indices;
			m->faces[i].indices = new uint[num_indices];
			memcpy(m->faces[i].indices, new_mesh->mFaces[i].mIndices, sizeof(uint) * num_indices);
		}
	}

	// normals
	if (new_mesh->HasNormals())
	{
		m->normals = new float[m->num_vertices * 3];
		memcpy(m->normals, new_mesh->mNormals, sizeof(float) * m->num_vertices * 3);
	}

	// texture coords (only one texture for now)
	if (new_mesh->HasTextureCoords(0))
	{
		m->texture_coords = new float[m->num_vertices * 3];
		memcpy(m->texture_coords, new_mesh->mTextureCoords[0], sizeof(float) * m->num_vertices * 3);
	}

	meshes.push_back(m);
	return m;
}