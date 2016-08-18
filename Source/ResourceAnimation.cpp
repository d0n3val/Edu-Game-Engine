#include "ResourceAnimation.h"
#include "Application.h"
#include "ModuleResources.h"
#include "LoaderAnimation.h"
#include "Config.h"

// ---------------------------------------------------------
ResourceAnimation::ResourceAnimation(UID uid) : Resource(uid, Resource::Type::animation)
{}

// ---------------------------------------------------------
ResourceAnimation::~ResourceAnimation()
{
	RELEASE_ARRAY(bone_keys);
}

// ---------------------------------------------------------
bool ResourceAnimation::LoadInMemory()
{
	return App->resources->GetAnimationLoader()->Load(this);
}

// ---------------------------------------------------------
void ResourceAnimation::Save(Config & config) const
{
	Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceAnimation::Load(const Config & config)
{
	Resource::Load(config);
}

// ---------------------------------------------------------
float ResourceAnimation::GetDurationInSecs() const
{
	return (float) (duration / ticks_per_second);
}

// ---------------------------------------------------------
void ResourceAnimation::FindBoneTransformation(float time, uint bone_index, float3 & pos, Quat & rot, float3 & scale) const
{
	if (bone_index >= num_keys)
		return;

	double ticks = time * ticks_per_second;
	bone_transform* transform = &bone_keys[bone_index];

	// find pos --------------------------
	uint i = 0;
	for (; i < transform->positions.count; ++i)
	{
		if (transform->positions.time[i] > ticks)
			break;
	}
	--i;
	pos.Set( transform->positions.value[i * 3 + 0],
		transform->positions.value[i * 3 + 1],
		transform->positions.value[i * 3 + 2]);

	// find rot ---------------------------
	i = 0;
	for (; i < transform->rotations.count; ++i)
	{
		if (transform->rotations.time[i] > ticks)
			break;
	}
	--i;
	rot.Set( transform->rotations.value[i * 3 + 0],
		transform->rotations.value[i * 3 + 1],
		transform->rotations.value[i * 3 + 2],
		transform->rotations.value[i * 3 + 3]);

	// find scale --------------------------
	i = 0;
	for (; i < transform->scales.count; ++i)
	{
		if (transform->scales.time[i] > ticks)
			break;
	}
	--i;
	scale.Set( transform->scales.value[i * 3 + 0],
		transform->scales.value[i * 3 + 1],
		transform->scales.value[i * 3 + 2]);

}
