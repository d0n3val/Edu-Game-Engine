#include "Globals.h"
#include "Application.h"
#include "ModuleMeshes.h"
#include "ModuleFileSystem.h"
#include "ResourceMesh.h"
#include "Glew/include/glew.h" // extensio lib
#include "OpenGL.h"
#include "Assimp/include/mesh.h"
#include "Math.h"

#include "mmgr/mmgr.h"

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
					  
	return true;
}

// Load new mesh
bool ModuleMeshes::Import(const aiMesh* new_mesh, string& output) const
{
	bool ret = false;

	if (new_mesh == nullptr)
		return ret;

	// Temporary object to make the load/Save process
	ResourceMesh m(0);

	// copy vertices
	m.num_vertices = new_mesh->mNumVertices;
	m.vertices = new float[m.num_vertices * 3];
	memcpy(m.vertices, new_mesh->mVertices, sizeof(float) * m.num_vertices * 3);
	LOG("New mesh with %d vertices", m.num_vertices);

	// copy faces
	if (new_mesh->HasFaces())
	{
		m.num_indices = new_mesh->mNumFaces * 3;
		m.indices = new uint[m.num_indices]; // assume each face is a triangle
		for (uint i = 0; i < new_mesh->mNumFaces; ++i)
		{
			if(new_mesh->mFaces[i].mNumIndices != 3)
				LOG("WARNING, geometry face with %d indices, all should be have 3!", new_mesh->mFaces[i].mNumIndices);

			memcpy(&m.indices[i*3], new_mesh->mFaces[i].mIndices, 3 * sizeof(uint));
		}
	}

	// normals
	if (new_mesh->HasNormals())
	{
		m.normals = new float[m.num_vertices * 3];
		memcpy(m.normals, new_mesh->mNormals, sizeof(float) * m.num_vertices * 3);
	}

	// colors
	if (new_mesh->HasVertexColors(0))
	{
		m.colors = new float[m.num_vertices * 3];
		memcpy(m.colors, new_mesh->mColors, sizeof(float) * m.num_vertices * 3);
	}

	// texture coords (only one texture for now)
	if (new_mesh->HasTextureCoords(0))
	{
		m.texture_coords = new float[m.num_vertices * 3];
		memcpy(m.texture_coords, new_mesh->mTextureCoords[0], sizeof(float) * m.num_vertices * 3);
	}

	// Generate AABB
	m.bbox.SetNegativeInfinity();
	m.bbox.Enclose((float3*) m.vertices, m.num_vertices);

	return Save(m, output);
}

bool ModuleMeshes::Load(ResourceMesh* resource)
{
	bool ret = false;

	if (resource == nullptr || resource->GetExportedFile() == nullptr)
		return ret;

	char* buffer;
	uint size = App->fs->Load(LIBRARY_MESH_FOLDER, resource->GetExportedFile(), &buffer);

	if (buffer != nullptr && size > 0)
	{
		char* cursor = buffer;

		// amount of indices / vertices / colors / normals / texture_coords
		uint ranges[5];
		uint bytes = sizeof(ranges);
		memcpy(ranges, cursor, bytes);

		resource->num_indices = ranges[0];
		resource->num_vertices = ranges[1];
		
		// Load indices
		cursor += bytes;
		bytes = sizeof(uint) * resource->num_indices;
		resource->indices = new uint[resource->num_indices];
		memcpy(resource->indices, cursor, bytes);

		// Load vertices
		cursor += bytes;
		bytes = sizeof(float) * resource->num_vertices * 3;
		resource->vertices = new float[resource->num_vertices * 3];
		memcpy(resource->vertices, cursor, bytes);

		// Load colors
		if (ranges[2] > 0)
		{
			cursor += bytes;
			resource->colors = new float[resource->num_vertices * 3];
			memcpy(resource->colors, cursor, bytes);
		}
		
		// Load normals
		if (ranges[3] > 0)
		{
			cursor += bytes;
			resource->normals = new float[resource->num_vertices * 3];
			memcpy(resource->normals, cursor, bytes);
		}

		// Load texture coords
		if (ranges[4] > 0)
		{
			cursor += bytes;
			resource->texture_coords = new float[resource->num_vertices * 3];
			memcpy(resource->texture_coords, cursor, bytes);
		}

		// AABB
		cursor += bytes;
		bytes = sizeof(AABB);
		memcpy(&resource->bbox.minPoint.x, cursor, bytes);

		RELEASE_ARRAY(buffer);

		GenerateVertexBuffer(resource); // we need to do this here ?

		ret = true;
	}

	return ret;
}

bool ModuleMeshes::Save(const ResourceMesh& mesh, string& output) const
{
	bool ret = false;

	// amount of indices / vertices / colors / normals / texture_coords / AABB
	uint ranges[5] = {
		mesh.num_indices,
		mesh.num_vertices,
		(mesh.colors) ? mesh.num_vertices : 0,
		(mesh.normals) ? mesh.num_vertices : 0,
		(mesh.texture_coords) ? mesh.num_vertices : 0
	};

	uint size = sizeof(ranges); 
	size += sizeof(uint) * mesh.num_indices;
	size += sizeof(float) * mesh.num_vertices * 3;
	if (mesh.colors != nullptr)
		size += sizeof(float) * mesh.num_vertices * 3;
	if (mesh.normals != nullptr)
		size += sizeof(float) * mesh.num_vertices * 3;
	if (mesh.texture_coords != nullptr)
		size += sizeof(float) * mesh.num_vertices * 3;
	size += sizeof(AABB);

	// allocate and fill
	char* data = new char[size];
	char* cursor = data;

	// First store ranges
	uint bytes = sizeof(ranges);
	memcpy(cursor, ranges, bytes);

	// Store indices
	cursor += bytes;
	bytes = sizeof(uint) * mesh.num_indices;
	memcpy(cursor, mesh.indices, bytes);

	// Store vertices
	cursor += bytes;
	bytes = sizeof(float) * mesh.num_vertices * 3;
	memcpy(cursor, mesh.vertices, bytes);

	// Store colors
	if (mesh.colors != nullptr)
	{
		cursor += bytes;
		memcpy(cursor, mesh.colors, bytes);
	}

	// Store normals
	if (mesh.normals != nullptr)
	{
		cursor += bytes;
		memcpy(cursor, mesh.normals, bytes);
	}

	// Store texture coords
	if (mesh.texture_coords != nullptr)
	{
		cursor += bytes;
		memcpy(cursor, mesh.texture_coords, bytes);
	}

	// Store AABB
	cursor += bytes;
	bytes = sizeof(AABB);
	memcpy(cursor, &mesh.bbox.minPoint.x, bytes);

	// We are ready to write the file
	ret = App->fs->SaveUnique(output, data, size, LIBRARY_MESH_FOLDER, "mesh", "edumesh");

	RELEASE_ARRAY(data);

	return ret;
}

void ModuleMeshes::GenerateVertexBuffer(const ResourceMesh* mesh)
{
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

	// Buffer for vertex colors
	if (mesh->colors != nullptr)
	{
		glGenBuffers (1, (GLuint*) &(mesh->vbo_colors));
		glBindBuffer (GL_ARRAY_BUFFER, mesh->vbo_colors);
		glBufferData (GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, mesh->colors, GL_STATIC_DRAW);
	}

	// Buffer for texture coords
	if (mesh->texture_coords != nullptr)
	{
		glGenBuffers(1, (GLuint*) &(mesh->vbo_texture_coords));
		glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo_texture_coords);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertices * 3, mesh->texture_coords, GL_STATIC_DRAW);
	}

	/* that when we switch to shaders
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
}

bool ModuleMeshes::LoadCube(ResourceMesh * resource)
{
	resource->file = "*Cube Preset*";
	resource->exported_file = "*Cube Preset*";

	//  indices -------------------------
	/* without extra vertices for texturing:
	uint indicescw[6 * 6] =
	{
		2, 7, 6, 2, 3, 7, // front
		3, 4, 7, 3, 0, 4, // right
		1, 4, 0, 1, 5, 4, // back
		2, 5, 1, 2, 6, 5, // left
		1, 3, 2, 1, 0, 3, // top
		7, 5, 6, 7, 4, 5  // bottom
	};*/

	uint indicescw[6 * 6] =
	{
		2, 7, 6, 2, 3, 7, // front
		11, 9, 10, 11, 8, 9, // right
		1, 4, 0, 1, 5, 4, // back
		15, 13, 12, 15, 14, 13, // left
		1, 3, 2, 1, 0, 3, // top
		7, 5, 6, 7, 4, 5  // bottom
	};

	resource->num_indices = 6 * 6;
	uint bytes = sizeof(uint) * resource->num_indices;
	resource->indices = new uint[resource->num_indices];
	memcpy(resource->indices, indicescw, bytes);

	//  vertices ------------------------
	float vertices[16 * 3] =
	{
		0.5f,  0.5f,  0.5f, // 0
	   -0.5f,  0.5f,  0.5f, // 1
	   -0.5f,  0.5f, -0.5f, // 2
		0.5f,  0.5f, -0.5f, // 3

		0.5f, -0.5f,  0.5f, // 4
	   -0.5f, -0.5f,  0.5f, // 5
	   -0.5f, -0.5f, -0.5f, // 6
		0.5f, -0.5f, -0.5f, // 7

		// add repeated vertices for proper texturing
		0.5f,  0.5f,  0.5f,  // 8
		0.5f, -0.5f,  0.5f,  // 9
		0.5f, -0.5f, -0.5f,  //10
		0.5f,  0.5f, -0.5f,  //11

	   -0.5f,  0.5f,  0.5f,  //12
	   -0.5f, -0.5f,  0.5f,  //13
	   -0.5f, -0.5f, -0.5f,  //14
	   -0.5f,  0.5f, -0.5f,  //15
	};

	resource->num_vertices = 16;
	bytes = sizeof(float) * resource->num_vertices * 3;
	resource->vertices = new float[resource->num_vertices* 3];
	memcpy(resource->vertices, vertices, bytes);

	// Load texture coords
	float texture_coords[16 * 3] =
	{
		1.f,  1.f,  0.f,
		0.f,  1.f,  0.f,
		0.f,  0.f,  0.f,
		1.f,  0.f,  0.f,

		1.f,  0.f,  0.f,
		0.f,  0.f,  0.f,
		0.f,  1.f,  0.f,
		1.f,  1.f,  0.f,
		
		// extra coords for left - right
		1.f,  1.f,  0.f,
		0.f,  1.f,  0.f,
		0.f,  0.f,  0.f,
		1.f,  0.f,  0.f,

		0.f,  1.f,  0.f,
		1.f,  1.f,  0.f,
		1.f,  0.f,  0.f,
		0.f,  0.f,  0.f,
	};

	resource->texture_coords = new float[resource->num_vertices * 3];
	memcpy(resource->texture_coords, texture_coords, bytes);

	// AABB
	resource->bbox = AABB(float3(-0.5f, -0.5f, -0.5f), float3(0.5f, 0.5f, 0.5f));
	
	// Now generate VBOs
	GenerateVertexBuffer(resource);

	return true;
}

bool ModuleMeshes::LoadSphere(ResourceMesh * resource)
{
	resource->file = "*Sphere Preset*";
	resource->exported_file = "*Sphere Preset*";

    std::vector<float3> vertices;
    std::vector<float3> normals;
    std::vector<float3> tex_coords;
    std::vector<uint> indicesVector;
    
    float latitudeBands = 20;
    float longitudeBands = 20;
    float radius = 1;
    
	for (float latNumber = 0; latNumber <= latitudeBands; latNumber++) {
		float theta = latNumber * PI / latitudeBands;
		float sinTheta = sin(theta);
		float cosTheta = cos(theta);

		for (float longNumber = 0; longNumber <= longitudeBands; longNumber++) {
			float phi = longNumber * 2 * PI / longitudeBands;
			float sinPhi = sin(phi);
			float cosPhi = cos(phi);

			float3 normal, vertex, tex_coord;

			normal.x = cosPhi * sinTheta;   // x
			normal.y = cosTheta;            // y
			normal.z = sinPhi * sinTheta;   // z
			tex_coord.x = 1 - (longNumber / longitudeBands); // u
			tex_coord.y = 1 - (latNumber / latitudeBands);   // v
			tex_coord.z = 0.f;
			vertex.x = radius * normal.x;
			vertex.y = radius * normal.y;
			vertex.z = radius * normal.z;

			normals.push_back(normal);
			tex_coords.push_back(tex_coord);
			vertices.push_back(vertex);
		}

		for (int latNumber = 0; latNumber < latitudeBands; latNumber++) {
			for (int longNumber = 0; longNumber < longitudeBands; longNumber++) {
				uint first = (latNumber * ((uint)longitudeBands + 1)) + longNumber;
				uint second = first + (uint)longitudeBands + 1;

				indicesVector.push_back(first);
				indicesVector.push_back(second);
				indicesVector.push_back(first + 1);

				indicesVector.push_back(second);
				indicesVector.push_back(second + 1);
				indicesVector.push_back(first + 1);

			}
		}
	}

	// Indicies -------------------------
	resource->num_indices = indicesVector.size();
	uint bytes = sizeof(uint) * resource->num_indices;
	resource->indices = new uint[resource->num_indices];
	memcpy(resource->indices, &indicesVector[0], bytes);
    
	// Vertices -------------------------
	resource->num_vertices = vertices.size();
	bytes = sizeof(float) * resource->num_vertices * 3;
	resource->vertices = new float[resource->num_vertices* 3];
	memcpy(resource->vertices, &vertices[0], bytes);

	// Normals --------------------------
	resource->normals = new float[resource->num_vertices * 3];
	memcpy(resource->normals, &normals[0], bytes);

	// Texture Coords -------------------
	resource->texture_coords = new float[resource->num_vertices * 3];
	memcpy(resource->texture_coords, &tex_coords[0], bytes);

	// AABB
	resource->bbox = AABB(float3(-radius, -radius, -radius), float3(radius, radius, radius));
	
	// Now generate VBOs
	GenerateVertexBuffer(resource);
	return true;
}

bool ModuleMeshes::LoadCone(ResourceMesh * resource)
{
	resource->file = "*Cone Preset*";
	resource->exported_file = "*Cone Preset*";
	BuildCone(resource, 2, 1, 0.f, 10);
	GenerateVertexBuffer(resource);
	return true;
}

bool ModuleMeshes::LoadCylinder(ResourceMesh * resource)
{
	resource->file = "*Cylinder Preset*";
	resource->exported_file = "*Cylinder Preset*";
	BuildCone(resource, 2, 1, 1, 10);
	GenerateVertexBuffer(resource);
	return true;
}

bool ModuleMeshes::LoadPyramid(ResourceMesh * resource)
{
	resource->file = "*Pyramid Preset*";
	resource->exported_file = "*Pyramid Preset*";
	BuildCone(resource, 2, 1, 0.f, 4);
	GenerateVertexBuffer(resource);
	return true;
}

// Procedurally generate cones cylinders or pyramids
// http://wiki.unity3d.com/index.php/ProceduralPrimitives
void ModuleMeshes::BuildCone(ResourceMesh * resource, float height, float bottom_radius, float top_radius, uint sides) const
{
	int nbHeightSeg = 1; // Not implemented yet
	int nbVerticesCap = sides + 1;
	height = height * 0.5f;
	float ybottom = -height;
	 
	// Vertices ------------------------------------------------------------

	// bottom + top + sides
	resource->num_vertices = nbVerticesCap + nbVerticesCap + sides * nbHeightSeg * 2 + 2;
	resource->vertices = new float[resource->num_vertices * 3];
	uint vert = 0;
	float _2pi = PI * 2.f;
	 
	// Bottom cap
	resource->vertices[vert * 3 + 0] = 0.f;
	resource->vertices[vert * 3 + 1] = ybottom;
	resource->vertices[vert * 3 + 2] = 0.f;
	vert++;
	while( vert <= sides )
	{
		float rad = (float)vert / sides * _2pi;
		resource->vertices[vert * 3 + 0] = cosf(rad) * bottom_radius;
		resource->vertices[vert * 3 + 1] = ybottom;
		resource->vertices[vert * 3 + 2] = sinf(rad) * bottom_radius;
		vert++;
	}
	 
	// Top cap
	resource->vertices[vert * 3 + 0] = 0.f;
	resource->vertices[vert * 3 + 1] = height;
	resource->vertices[vert * 3 + 2] = 0.f;
	while (vert <= sides * 2 + 1)
	{
		float rad = (float)(vert - sides - 1)  / sides * _2pi;
		resource->vertices[vert * 3 + 0] = cosf(rad) * top_radius;
		resource->vertices[vert * 3 + 1] = height;
		resource->vertices[vert * 3 + 2] = sinf(rad) * top_radius;
		vert++;
	}
	 
	// Sides
	int v = 0;
	while (vert <= (resource->num_vertices) - 2 )
	{
		float rad = (float)v / sides * _2pi;
		resource->vertices[vert * 3 + 0] = cosf(rad) * top_radius;
		resource->vertices[vert * 3 + 1] = height;
		resource->vertices[vert * 3 + 2] = sinf(rad) * top_radius;
		vert++;
		resource->vertices[vert * 3 + 0] = cosf(rad) * bottom_radius;
		resource->vertices[vert * 3 + 1] = ybottom;
		resource->vertices[vert * 3 + 2] = sinf(rad) * bottom_radius;
		vert++;
		v++;
	}

	// Normals ------------------------------------------------------------

	// bottom + top + sides
	resource->normals = new float[resource->num_vertices * 3];
	vert = 0;
	 
	// Bottom cap
	while( vert  <= sides )
	{
		resource->normals[vert * 3 + 0] = 0.f;
		resource->normals[vert * 3 + 1] = -1.f;
		resource->normals[vert * 3 + 2] = 0.f;
		vert++;
	}
	 
	// Top cap
	while( vert <= sides * 2 + 1 )
	{
		resource->normals[vert * 3 + 0] = 0.f;
		resource->normals[vert * 3 + 1] = 1.f;
		resource->normals[vert * 3 + 2] = 0.f;
		vert++;
	}
	 
	// Sides
	v = 0;
	while (vert <= resource->num_vertices - 2 )
	{			
		float rad = (float)v / sides * _2pi;
		float cos = cosf(rad);
		float sin = sinf(rad);
	 
		resource->normals[vert * 3 + 0] = cos;
		resource->normals[vert * 3 + 1] = 0.f;
		resource->normals[vert * 3 + 2] = sin;
		vert++;
		resource->normals[vert * 3 + 0] = cos;
		resource->normals[vert * 3 + 1] = 0.f;
		resource->normals[vert * 3 + 2] = sin;
		vert++;
		v++;
	}

	// Texture Coords ------------------------------------------------

	resource->texture_coords = new float[resource->num_vertices * 3];
	 
	// Bottom cap
	uint u = 0;
	resource->texture_coords[u * 3 + 0] = 0.5f;
	resource->texture_coords[u * 3 + 1] = 0.5f;
	resource->texture_coords[u * 3 + 2] = 0.0f;
	u++;
	while (u <= sides)
	{
	    float rad = (float)u / sides * _2pi;
		resource->texture_coords[u * 3 + 0] = cosf(rad) * 0.5f + 0.5f;
		resource->texture_coords[u * 3 + 1] = sinf(rad) * 0.5f + 0.5f;
		resource->texture_coords[u * 3 + 2] = 0.f;
	    u++;
	}
	 
	// Top cap
	resource->texture_coords[u * 3 + 0] = 0.5f;
	resource->texture_coords[u * 3 + 1] = 0.5f;
	resource->texture_coords[u * 3 + 2] = 0.0f;
	u++;
	while (u <= sides * 2 + 1)
	{
	    float rad = (float)u / sides * _2pi;
		resource->texture_coords[u * 3 + 0] = cosf(rad) * 0.5f + 0.5f;
		resource->texture_coords[u * 3 + 1] = sinf(rad) * 0.5f + 0.5f;
		resource->texture_coords[u * 3 + 2] = 0.f;
	    u++;
	}
	 
	// Sides
	int u_sides = 0;
	while (u <= resource->num_vertices - 2)
	{
		float t = (float)u_sides * 4 / sides;
		resource->texture_coords[u * 3 + 0] = t;
		resource->texture_coords[u * 3 + 1] = 1.f;
		resource->texture_coords[u * 3 + 2] = 0.f;
		u++;
		resource->texture_coords[u * 3 + 0] = t;
		resource->texture_coords[u * 3 + 1] = 0.f;
		resource->texture_coords[u * 3 + 2] = 0.f;
		u++;
		u_sides++;
	}

	// Indices -------------------------------------------------------

	uint nbTriangles = sides + sides + sides * 2;
	resource->num_indices = (nbTriangles) * 3 +3;
	resource->indices = new uint[resource->num_indices];
	 
	// Bottom cap
	uint tri = 0;
	int i = 0;
	while (tri < sides - 1)
	{
		resource->indices[ i + 0 ] = 0;
		resource->indices[ i + 1 ] = tri + 1;
		resource->indices[ i + 2 ] = tri + 2;
		tri++;
		i += 3;
	}
	resource->indices[i + 0] = 0;
	resource->indices[i + 1] = tri + 1;
	resource->indices[i + 2] = 1;
	tri++;
	i += 3;
	 
	// Top cap
	while (tri < sides * 2)
	{
		resource->indices[i + 0] = tri + 2;
		resource->indices[i + 1] = tri + 1;
		resource->indices[i + 2] = nbVerticesCap;
		tri++;
		i += 3;
	}
	 
	resource->indices[i + 0] = nbVerticesCap + 1;
	resource->indices[i + 1] = tri + 1;
	resource->indices[i + 2] = nbVerticesCap;		
	tri++;
	i += 3;
	tri++;
	 
	// Sides
	while( tri <= nbTriangles)
	{
		resource->indices[ i + 0 ] = tri + 2;
		resource->indices[ i + 1 ] = tri + 1;
		resource->indices[ i + 2 ] = tri + 0;
		tri++;
		i += 3;
	 
		resource->indices[ i + 0 ] = tri + 1;
		resource->indices[ i + 1 ] = tri + 2;
		resource->indices[ i + 2 ] = tri + 0;
		tri++;
		i += 3;
	}

	// AABB
	float radius = MAX(bottom_radius, top_radius);
	resource->bbox = AABB(float3(-radius, ybottom, -radius), float3(radius, height, radius));
}