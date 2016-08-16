#include "Globals.h"
#include "Application.h"
#include "LoaderBone.h"
#include "ModuleFileSystem.h"
#include "ResourceBone.h"
#include "Assimp/include/mesh.h"

#include "mmgr/mmgr.h"

using namespace std;

LoaderBone::LoaderBone() 
{}

LoaderBone::~LoaderBone()
{}

// Load new mesh
bool LoaderBone::Import(const aiBone* new_bone, UID mesh, string& output) const
{
	bool ret = false;

	if (new_bone == nullptr)
		return ret;

	// Temporary object to make the load/Save process
	ResourceBone bone(0);

	bone.uid_mesh = mesh;
	bone.num_weigths = new_bone->mNumWeights;
	memcpy(bone.offset.v, &new_bone->mOffsetMatrix.a1, sizeof(float) * 16);

	bone.weigth_indices = new uint[bone.num_weigths];
	bone.weigths = new float[bone.num_weigths];

	for (uint k = 0; k < bone.num_weigths; ++k)
	{
		bone.weigth_indices[k] = new_bone->mWeights[k].mVertexId;
		bone.weigths[k] = new_bone->mWeights[k].mWeight;
	}

	return Save(bone, output);
}

bool LoaderBone::Load(ResourceBone* resource) const
{
	bool ret = false;

	if (resource == nullptr || resource->GetExportedFile() == nullptr)
		return ret;

	char* buffer;
	uint sizef = App->fs->Load(LIBRARY_BONE_FOLDER, resource->GetExportedFile(), &buffer);

	if (buffer != nullptr && sizef > 0)
	{

		char* cursor = buffer;

		// See Save() method for format

		// Load mesh UID
		uint bytes = sizeof(UID);
		memcpy(&resource->uid_mesh, cursor, bytes);
		
		// Load offset matrix
		cursor += bytes;
		bytes = sizeof(resource->offset);
		memcpy(resource->offset.v, cursor, bytes);

		// Load num_weigths
		cursor += bytes;
		bytes = sizeof(resource->num_weigths);
		memcpy(&resource->num_weigths, cursor, bytes);

		uint size = sizeof(resource->uid_mesh);
		size += sizeof(resource->offset);
		size += sizeof(resource->num_weigths);
		size += sizeof(uint) * resource->num_weigths;
		size += sizeof(float) * resource->num_weigths;
		// Allocate mem
		resource->weigth_indices = new uint[resource->num_weigths];
		resource->weigths = new float[resource->num_weigths];

		// read indices
		cursor += bytes;
		bytes = sizeof(uint) * resource->num_weigths;
		memcpy(resource->weigth_indices, cursor, bytes);

		// read weigths
		cursor += bytes;
		bytes = sizeof(float) * resource->num_weigths;
		memcpy(resource->weigths, cursor, bytes);

		RELEASE_ARRAY(buffer);

		ret = true;
	}

	return ret;
}

bool LoaderBone::Save(const ResourceBone& bone, string& output) const
{
	bool ret = false;

	// Format: mesh UID + 16 float matrix + num_weigths uint + indices uint * num_weight + weight float * num_weights
	uint size = sizeof(bone.uid_mesh);
	size += sizeof(bone.offset);
	size += sizeof(bone.num_weigths);
	size += sizeof(uint) * bone.num_weigths;
	size += sizeof(float) * bone.num_weigths;

	// allocate mem
	char* data = new char[size];
	char* cursor = data;

	// store mesh UID
	uint bytes = sizeof(bone.uid_mesh);
	memcpy(cursor, &bone.uid_mesh, bytes);

	// store offset matrix
	cursor += bytes;
	bytes = sizeof(bone.offset);
	memcpy(cursor, &bone.offset.v, bytes);

	// store num_weights
	cursor += bytes;
	bytes = sizeof(bone.num_weigths);
	memcpy(cursor, &bone.num_weigths, bytes);

	// store indices
	cursor += bytes;
	bytes = sizeof(uint) * bone.num_weigths;
	memcpy(cursor, bone.weigth_indices, bytes);

	// store weights
	cursor += bytes;
	bytes = sizeof(float) * bone.num_weigths;
	memcpy(cursor, bone.weigths, bytes);

	// We are ready to write the file
	ret = App->fs->SaveUnique(output, data, size, LIBRARY_BONE_FOLDER, "bone", "edubone");

	RELEASE_ARRAY(data);
	
	return ret;
}
