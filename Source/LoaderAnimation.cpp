#include "Globals.h"
#include "Application.h"
#include "LoaderAnimation.h"
#include "ModuleFileSystem.h"
#include "ResourceAnimation.h"
#include "Application.h"
#include "ModuleSceneLoader.h"
#include "Assimp/include/anim.h"
#include "Math.h"


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

	// Happened once importeing a fbx, so just in case
	if (new_anim->mNumChannels == 0)
		return ret;

	// Temporary object to make the load/Save process
	ResourceAnimation anim(0);

	anim.name = new_anim->mName.C_Str();
	anim.duration = new_anim->mDuration;
	anim.ticks_per_second = new_anim->mTicksPerSecond;

	// bone transformations
	anim.num_keys = new_anim->mNumChannels;
	anim.bone_keys = new ResourceAnimation::bone_transform[new_anim->mNumChannels];
	for (uint i = 0; i < new_anim->mNumChannels; ++i)
		ImportBoneTransform(new_anim->mChannels[i], anim.bone_keys[i]);

	return Save(anim, output);
}

void LoaderAnimation::ImportBoneTransform(const aiNodeAnim * node, ResourceAnimation::bone_transform& bone) const
{
	// copy bone name
	bone.bone_name = node->mNodeName.C_Str();

	// Import positions
	bone.positions.Alloc(node->mNumPositionKeys, 3);

	for (uint k = 0; k < node->mNumPositionKeys; ++k)
	{
		aiVectorKey translation = node->mPositionKeys[k];
		bone.positions.time[k] = translation.mTime;
		bone.positions.value[k*3+0] = translation.mValue.x;
		bone.positions.value[k*3+1] = translation.mValue.y;
		bone.positions.value[k*3+2] = translation.mValue.z;
	}

	// Import rotation
	bone.rotations.Alloc(node->mNumRotationKeys, 4);

	for (uint k = 0; k < node->mNumRotationKeys; ++k)
	{
		aiQuatKey rotation = node->mRotationKeys[k];
		bone.rotations.time[k] = rotation.mTime;
		bone.rotations.value[k*4+0] = rotation.mValue.x;
		bone.rotations.value[k*4+1] = rotation.mValue.y;
		bone.rotations.value[k*4+2] = rotation.mValue.z;
		bone.rotations.value[k*4+3] = rotation.mValue.w;
	}

	// Import scales
	bone.scales.Alloc(node->mNumScalingKeys, 3);

	for (uint k = 0; k < node->mNumScalingKeys; ++k)
	{
		aiVectorKey scale = node->mScalingKeys[k];
		bone.scales.time[k] = scale.mTime;
		// I've ssen most scales with numbers very close to one, might be useful
		// to check for that epsilon and just use (1,1,1) ... bloody floats :(
		bone.scales.value[k*3+0] = scale.mValue.x;
		bone.scales.value[k*3+1] = scale.mValue.y;
		bone.scales.value[k*3+2] = scale.mValue.z;
	}
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

		// Load amount Bone transformations
		cursor += bytes;
		bytes = sizeof(resource->num_keys);
		memcpy(&resource->num_keys, cursor, bytes);

		resource->bone_keys = new ResourceAnimation::bone_transform[resource->num_keys];

		char buff[4096];
		for (uint i = 0; i < resource->num_keys; ++i)
		{
			ResourceAnimation::bone_transform* bone = &resource->bone_keys[i];
			uint count = 0;

			// load bone name size
			cursor += bytes;
			bytes = sizeof(count);
			memcpy(&count, cursor, bytes);

			// load bone name
			cursor += bytes;
			bytes = sizeof(char) * count + 1;
			memcpy(buff, cursor, bytes);
			bone->bone_name = buff;

			// load num_positions -------------------------------
			cursor += bytes;
			bytes = sizeof(count);
			memcpy(&count, cursor, bytes);
			bone->positions.Alloc(count, 3);

			// load position times
			cursor += bytes;
			bytes = sizeof(double) * count;
			memcpy(bone->positions.time, cursor, bytes);

			// load position values
			cursor += bytes;
			bytes = sizeof(float) * 3 * count;
			memcpy(bone->positions.value, cursor, bytes);

			// load num rotations -------------------------------
			count = 0;
			cursor += bytes;
			bytes = sizeof(count);
			memcpy(&count, cursor, bytes);
			bone->rotations.Alloc(count, 4);

			// load rotation times
			cursor += bytes;
			bytes = sizeof(double) * count;
			memcpy(bone->rotations.time, cursor, bytes);

			// load rotation values
			cursor += bytes;
			bytes = sizeof(float) * 4 * count;
			memcpy(bone->rotations.value, cursor, bytes);

			// load num_scales -------------------------------
			count = 0;
			cursor += bytes;
			bytes = sizeof(count);
			memcpy(&count, cursor, bytes);
			bone->scales.Alloc(count, 3);

			// load position times
			cursor += bytes;
			bytes = sizeof(double) * count;
			memcpy(bone->scales.time, cursor, bytes);

			// load position values
			cursor += bytes;
			bytes = sizeof(float) * 3 * count;
			memcpy(bone->scales.value, cursor, bytes);
		}

		RELEASE_ARRAY(buffer);

		ret = true;
	}

	return ret;
}

bool LoaderAnimation::Save(const ResourceAnimation& anim, string& output) const
{
	bool ret = false;

	// Format: uint name size, name chars + 1, double duration double ticks_per_second
	// uint count transforms, then for each: UID bone, num_pos, float3[], num_rot, Quat[], num_scale, float3[]
	uint size = sizeof(char) * ANIM_NAME_SIZE;
	size += sizeof(anim.duration);
	size += sizeof(anim.ticks_per_second);
	size += sizeof(anim.num_keys);

	for (uint i = 0; i < anim.num_keys; ++i)
	{
		size += sizeof(uint);
		size += sizeof(char) * anim.bone_keys[i].bone_name.size() + 1;

		size += sizeof(anim.bone_keys[i].positions.count);
		size += sizeof(double) * anim.bone_keys[i].positions.count;
		size += sizeof(float) * 3 * anim.bone_keys[i].positions.count;

		size += sizeof(anim.bone_keys[i].rotations.count);
		size += sizeof(double) * anim.bone_keys[i].rotations.count;
		size += sizeof(float) * 4 * anim.bone_keys[i].rotations.count;

		size += sizeof(anim.bone_keys[i].scales.count);
		size += sizeof(double) * anim.bone_keys[i].scales.count;
		size += sizeof(float) * 3 * anim.bone_keys[i].scales.count;
	}

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

	// store num_transforms
	cursor += bytes;
	bytes = sizeof(anim.num_keys);
	memcpy(cursor, &anim.num_keys, bytes);

	for (uint i = 0; i < anim.num_keys; ++i)
	{
		// name size
		cursor += bytes;
		bytes = sizeof(uint);
		uint name_size = anim.bone_keys[i].bone_name.size();
		memcpy(cursor, &name_size, bytes);

		// name
		cursor += bytes;
		bytes = sizeof(char) * name_size + 1;
		memcpy(cursor, anim.bone_keys[i].bone_name.c_str(), bytes);
		LOG("Adding animation for bone %s", anim.bone_keys[i].bone_name.c_str());

		// store num_positions ---------------------------
		cursor += bytes;
		bytes = sizeof(anim.bone_keys[i].positions.count);
		memcpy(cursor, &anim.bone_keys[i].positions.count, bytes);

		// store times for positions
		cursor += bytes;
		bytes = sizeof(double) * anim.bone_keys[i].positions.count;
		memcpy(cursor, anim.bone_keys[i].positions.time, bytes);

		// store positions
		cursor += bytes;
		bytes = sizeof(float) * 3 * anim.bone_keys[i].positions.count;
		memcpy(cursor, anim.bone_keys[i].positions.value, bytes);

		// store num_rotations ----------------------------
		cursor += bytes;
		bytes = sizeof(anim.bone_keys[i].rotations.count);
		memcpy(cursor, &anim.bone_keys[i].rotations.count, bytes);

		// store times for rotations
		cursor += bytes;
		bytes = sizeof(double) * anim.bone_keys[i].rotations.count;
		memcpy(cursor, anim.bone_keys[i].rotations.time, bytes);

		// store positions
		cursor += bytes;
		bytes = sizeof(float) * 4 * anim.bone_keys[i].rotations.count;
		memcpy(cursor, anim.bone_keys[i].rotations.value, bytes);

		// store num_scales ---------------------------
		cursor += bytes;
		bytes = sizeof(anim.bone_keys[i].scales.count);
		memcpy(cursor, &anim.bone_keys[i].scales.count, bytes);

		// store times for scales
		cursor += bytes;
		bytes = sizeof(double) * anim.bone_keys[i].scales.count;
		memcpy(cursor, anim.bone_keys[i].scales.time, bytes);

		// store scales
		cursor += bytes;
		bytes = sizeof(float) * 3 * anim.bone_keys[i].scales.count;
		memcpy(cursor, anim.bone_keys[i].scales.value, bytes);
	}

	// We are ready to write the file
	ret = App->fs->SaveUnique(output, data, size, LIBRARY_ANIMATION_FOLDER, "anim", "eduanim");

	RELEASE_ARRAY(data);
	return ret;
}


