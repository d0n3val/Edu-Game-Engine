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
	LOG("New mesh with %d vertices", m->num_vertices);

	// copy faces
	if (new_mesh->HasFaces())
	{
		m->num_indices = new_mesh->mNumFaces * 3;
		m->indices = new uint[m->num_indices]; // assume each face is a triangle
		for (uint i = 0; i < new_mesh->mNumFaces; ++i)
		{
			if(new_mesh->mFaces[i].mNumIndices != 3)
				LOG("WARNING, geometry face with %d indices, all should be have 3!", new_mesh->mFaces[i].mNumIndices);

			memcpy(&m->indices[i*3], new_mesh->mFaces[i].mIndices, 3 * sizeof(uint));
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

	GenerateVertexBuffer(m);
	return m;
}

uint ModuleMeshes::GenerateVertexBuffer(const Mesh* mesh)
{
	uint ret = 0;

	// Generate VBO to send all this mesh information to the graphics card

	// Buffer for vertices
	glGenBuffers (1, (GLuint*) &(mesh->vbo_vertices));
	glBindBuffer (GL_ARRAY_BUFFER, mesh->vbo_vertices);
	glBufferData (GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, mesh->vertices, GL_STATIC_DRAW);

	// Buffer for indices
	glGenBuffers (1, (GLuint*) &(mesh->vbo_indices));
	glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mesh->vbo_indices);
	glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * mesh->num_indices, mesh->indices, GL_STATIC_DRAW);

	// Buffer for normals
	if (mesh->normals != nullptr)
	{
		glGenBuffers (1, (GLuint*) &(mesh->vbo_normals));
		glBindBuffer (GL_ARRAY_BUFFER, mesh->vbo_normals);
		glBufferData (GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, mesh->normals, GL_STATIC_DRAW);
	}

	// Buffer for texture coords
	glGenBuffers (1, (GLuint*) &(mesh->vbo_texture_coords));
	glBindBuffer (GL_ARRAY_BUFFER, mesh->vbo_texture_coords);
	glBufferData (GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, mesh->texture_coords, GL_STATIC_DRAW);

	/*
	// Now we pack all buffers using a VAO (Vertex Attribute Object)
	glGenVertexArrays(1, &ret); // generate one VAO and fill its id in ret
	glBindVertexArray(ret);	// start using this VAO

	// From OpenGL 3.2+ on we can use VAO (but this ill require a shader) --- 
	// Add our vertices on position 0
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_vertices);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Add our normals on position 1
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_normals);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	// Add our texture coordinates on position 2
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_texture_coords);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	LOG("New VAO with id %u", ret);
	*/
	return ret;
}