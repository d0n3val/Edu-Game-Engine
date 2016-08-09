#include "Globals.h"
#include "Application.h"
#include "ModuleMeshes.h"
#include "ModuleFileSystem.h"
#include "Glew/include/glew.h" // extensio lib
#include "gl/GL.h"
#include "Assimp/include/mesh.h"

using namespace std;

ModuleMeshes::ModuleMeshes(bool start_enabled) : Module("Meshes", start_enabled)
{
}

// Destructor
ModuleMeshes::~ModuleMeshes()
{
}

// Called before render is available
bool ModuleMeshes::Init(Config* config)
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

// Load new mesh
const char* ModuleMeshes::Import(const aiMesh* new_mesh) const
{
	if (new_mesh == nullptr)
		return nullptr;

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

	const char* filename = Save(m);
	RELEASE(m);

	return filename;
}

const Mesh * ModuleMeshes::Load(const char * file)
{
	if (file == nullptr)
		return nullptr;

	Mesh* mesh = nullptr;

	char* buffer;
	uint size = App->fs->Load(file, &buffer);

	if (size > 0)
	{
		char* cursor = buffer;
		mesh = new Mesh;

		// amount of indices / vertices / colors / normals / texture_coords
		uint ranges[5];
		uint bytes = sizeof(ranges);
		memcpy(ranges, cursor, bytes);

		mesh->num_indices = ranges[0];
		mesh->num_vertices = ranges[1];
		
		// Load indices
		cursor += bytes;
		bytes = sizeof(uint) * mesh->num_indices;
		mesh->indices = new uint[mesh->num_indices];
		memcpy(mesh->indices, cursor, bytes);

		// Load vertices
		cursor += bytes;
		bytes = sizeof(float) * mesh->num_vertices * 3;
		mesh->vertices = new float[mesh->num_vertices * 3];
		memcpy(mesh->vertices, cursor, bytes);

		// Load colors
		if (ranges[2] > 0)
		{
			cursor += bytes;
			mesh->colors = new float[mesh->num_vertices * 3];
			memcpy(mesh->colors, cursor, bytes);
		}
		
		// Load normals
		if (ranges[3] > 0)
		{
			cursor += bytes;
			mesh->normals = new float[mesh->num_vertices * 3];
			memcpy(mesh->normals, cursor, bytes);
		}

		// Load texture coords
		if (ranges[4] > 0)
		{
			cursor += bytes;
			mesh->texture_coords = new float[mesh->num_vertices * 3];
			memcpy(mesh->texture_coords, cursor, bytes);
		}

		RELEASE(buffer);

		meshes.push_back(mesh);
		GenerateVertexBuffer(mesh); // we need to do this here ?
	}

	return mesh;
}

const char * ModuleMeshes::Save(const Mesh * mesh) const
{
	// amount of indices / vertices / colors / normals / texture_coords
	uint ranges[5] = {
		mesh->num_indices,
		mesh->num_vertices,
		(mesh->colors) ? mesh->num_vertices : 0,
		(mesh->normals) ? mesh->num_vertices : 0,
		(mesh->texture_coords) ? mesh->num_vertices : 0
	};

	uint size = sizeof(ranges); 
	size += sizeof(uint) * mesh->num_indices;
	size += sizeof(float) * mesh->num_vertices * 3;
	if (mesh->colors != nullptr)
		size += sizeof(float) * mesh->num_vertices * 3;
	if (mesh->normals != nullptr)
		size += sizeof(float) * mesh->num_vertices * 3;
	if (mesh->texture_coords != nullptr)
		size += sizeof(float) * mesh->num_vertices * 3;

	// allocate and fill
	char* data = new char[size];
	char* cursor = data;

	// First store ranges
	uint bytes = sizeof(ranges);
	memcpy(cursor, ranges, bytes);

	// Store indices
	cursor += bytes;
	bytes = sizeof(uint) * mesh->num_indices;
	memcpy(cursor, mesh->indices, bytes);

	// Store vertices
	cursor += bytes;
	bytes = sizeof(float) * mesh->num_vertices * 3;
	memcpy(cursor, mesh->vertices, bytes);

	// Store colors
	if (mesh->colors != nullptr)
	{
		cursor += bytes;
		memcpy(cursor, mesh->colors, bytes);
	}

	// Store normals
	if (mesh->normals != nullptr)
	{
		cursor += bytes;
		memcpy(cursor, mesh->normals, bytes);
	}

	// Store texture coords
	if (mesh->texture_coords != nullptr)
	{
		cursor += bytes;
		memcpy(cursor, mesh->texture_coords, bytes);
	}

	// We are ready to write the file
	static int f = 0;
	static char file[80];

	sprintf_s(file, 80, "assets/meshes/mesh_%i.mesh", ++f);
	App->fs->Save(file, data, size);

	RELEASE(data);

	return file;
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

