#include "ResourceMesh.h"
#include "Application.h"
#include "ModuleMeshes.h"

// ---------------------------------------------------------
ResourceMesh::ResourceMesh(UID uid) : Resource(uid, Resource::Type::mesh)
{}

// ---------------------------------------------------------
ResourceMesh::~ResourceMesh()
{
	// TODO: deallocate opengl buffers
	RELEASE_ARRAY(indices);
	RELEASE_ARRAY(vertices);
	RELEASE_ARRAY(colors);
	RELEASE_ARRAY(normals);
	RELEASE_ARRAY(texture_coords);
}

// ---------------------------------------------------------
bool ResourceMesh::LoadInMemory()
{
	return App->meshes->Load(this);
}

// ---------------------------------------------------------
void ResourceMesh::Save(Config & config) const
{
	Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceMesh::Load(const Config & config)
{
	Resource::Load(config);
}

// ---------------------------------------------------------
void ResourceMesh::DeepCopyFrom(const ResourceMesh * original)
{
	if (original != nullptr)
	{
		num_indices = original->num_indices;
		num_vertices = original->num_vertices;

		indices = new uint[num_indices];
		memcpy(indices, original->indices, num_indices * sizeof(uint));

		vertices = new float[num_vertices * 3];
		memcpy(vertices, original->vertices, num_vertices * sizeof(float) * 3);

		if (normals != nullptr)
		{
			normals = new float[num_vertices * 3];
			memcpy(normals, original->normals, num_vertices * sizeof(float) * 3);
		}

		if (colors != nullptr)
		{
			colors = new float[num_vertices * 3];
			memcpy(normals, original->colors, num_vertices * sizeof(float) * 3);
		}

		if (texture_coords != nullptr)
		{
			texture_coords = new float[num_vertices * 3];
			memcpy(texture_coords, original->texture_coords, num_vertices * sizeof(float) * 3);
		}
	}
}
