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
