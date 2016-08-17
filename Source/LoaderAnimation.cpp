#include "Globals.h"
#include "Application.h"
#include "LoaderAnimation.h"
#include "ModuleFileSystem.h"
#include "ResourceAnimation.h"
#include "Assimp/include/anim.h"

#define ANIM_NAME_SIZE 25

using namespace std;

LoaderAnimation::LoaderAnimation() 
{}

LoaderAnimation::~LoaderAnimation()
{}

// Load new mesh
bool LoaderAnimation::Import(const aiAnimation* new_anim, UID mesh, string& output) const
{
	bool ret = false;

	if (new_anim == nullptr)
		return ret;

	// Temporary object to make the load/Save process
	ResourceAnimation anim(0);

	anim.name = new_anim->mName.C_Str();
	anim.duration = new_anim->mDuration;
	anim.ticks_per_second = new_anim->mTicksPerSecond;

	return Save(anim, output);
}

bool LoaderAnimation::Load(ResourceAnimation* resource) const
{
	bool ret = false;

	if (resource == nullptr || resource->GetExportedFile() == nullptr)
		return ret;

	char* buffer;
	uint sizef = App->fs->Load(LIBRARY_ANIMATION_FOLDER, resource->GetExportedFile(), &buffer);

	if (buffer != nullptr && sizef > 0)
	{
		char* cursor = buffer;

		// See Save() method for format

		// Load name
		uint bytes = sizeof(char) * ANIM_NAME_SIZE;
		char name[ANIM_NAME_SIZE];
		memcpy(&name, cursor, bytes);
		resource->name = name;
		
		// Load duration
		cursor += bytes;
		bytes = sizeof(resource->duration);
		memcpy(&resource->duration, cursor, bytes);

		// Load ticks per second
		cursor += bytes;
		bytes = sizeof(resource->ticks_per_second);
		memcpy(&resource->ticks_per_second, cursor, bytes);

		RELEASE_ARRAY(buffer);

		ret = true;
	}

	return ret;
}

bool LoaderAnimation::Save(const ResourceAnimation& anim, string& output) const
{
	bool ret = false;

	// Format: name char[25], double duration double ticks_per_second
	uint size = sizeof(sizeof(char) * ANIM_NAME_SIZE);
	size += sizeof(anim.duration);
	size += sizeof(anim.ticks_per_second);

	// allocate mem
	char* data = new char[size];
	char* cursor = data;

	// store name
	uint bytes = sizeof(char) * ANIM_NAME_SIZE;
	char name[ANIM_NAME_SIZE];
	memset(name, 0, sizeof(char) * ANIM_NAME_SIZE);
	strcpy_s(name, ANIM_NAME_SIZE, anim.name.c_str());

	memcpy(cursor, name, bytes);

	// store duration
	cursor += bytes;
	bytes = sizeof(anim.duration);
	memcpy(cursor, &anim.duration, bytes);

	// store num_weights
	cursor += bytes;
	bytes = sizeof(anim.ticks_per_second);
	memcpy(cursor, &anim.ticks_per_second, bytes);

	// We are ready to write the file
	ret = App->fs->SaveUnique(output, data, size, LIBRARY_ANIMATION_FOLDER, "anim", "eduanim");

	RELEASE_ARRAY(data);
	return ret;
}
