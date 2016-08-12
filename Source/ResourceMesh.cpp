#include "ResourceMesh.h"

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
void ResourceMesh::Save(Config & config) const
{
	Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceMesh::Load(const Config & config)
{
	Resource::Load(config);
}
