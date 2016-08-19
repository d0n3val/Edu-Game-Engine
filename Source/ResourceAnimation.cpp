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
void ResourceAnimation::FindBoneTransformation(float time, uint bone_index, float3 & pos, Quat & rot, float3 & scale, bool interpolate) const
{
	if (bone_index >= num_keys)
		return;

	double ticks = time * ticks_per_second;
	bone_transform* transform = &bone_keys[bone_index];
	double time_slice;
	float3 vprev, vnext;
	Quat qprev, qnext;
	uint prev, next;

	// find pos --------------------------
	prev = transform->positions.count - 1;
	next = 0;
	for (uint i = 0; i < transform->positions.count; ++i)
	{
		if (transform->positions.time[i] > ticks)
		{
			prev = i - 1;
			next = i;
			break;
		}
	}

	vprev.Set(
		transform->positions.value[prev * 3 + 0],
		transform->positions.value[prev * 3 + 1],
		transform->positions.value[prev * 3 + 2]);

	vnext.Set(
		transform->positions.value[next * 3 + 0],
		transform->positions.value[next * 3 + 1],
		transform->positions.value[next * 3 + 2]);

	if(interpolate == false || vprev.Equals(vnext) == true )
		pos = vprev;
	else
	{
		time_slice = (ticks - transform->positions.time[prev]) / (transform->positions.time[next] - transform->positions.time[prev]);
		pos = float3::Lerp(vprev, vnext, (float) time_slice);
	}

	// find rot ---------------------------
	prev = transform->rotations.count - 1;
	next = 0;
	for (uint i = 0; i < transform->rotations.count; ++i)
	{
		if (transform->rotations.time[i] > ticks)
		{
			prev = i - 1;
			next = i;
			break;
		}
	}

	qprev.Set(
		transform->rotations.value[prev * 4 + 0],
		transform->rotations.value[prev * 4 + 1],
		transform->rotations.value[prev * 4 + 2],
		transform->rotations.value[prev * 4 + 3]);

	qnext.Set(
		transform->rotations.value[next * 4 + 0],
		transform->rotations.value[next * 4 + 1],
		transform->rotations.value[next * 4 + 2],
		transform->rotations.value[next * 4 + 3]);

	if(interpolate == false || qprev.Equals(qnext) == true )
		rot = qprev;
	else
	{
		time_slice = (ticks - transform->rotations.time[prev]) / (transform->rotations.time[next] - transform->rotations.time[prev]);
		rot = Quat::Slerp(qprev, qnext, (float) time_slice);
	}

	// find scale --------------------------
	prev = transform->scales.count - 1;
	next = 0;
	for (uint i = 0; i < transform->scales.count; ++i)
	{
		if (transform->scales.time[i] > ticks)
		{
			prev = i - 1;
			next = i;
			break;
		}
	}

	vprev.Set(
		transform->scales.value[prev * 3 + 0],
		transform->scales.value[prev * 3 + 1],
		transform->scales.value[prev * 3 + 2]);

	vnext.Set(
		transform->scales.value[next * 3 + 0],
		transform->scales.value[next * 3 + 1],
		transform->scales.value[next * 3 + 2]);

	if(interpolate == false || vprev.Equals(vnext) == true )
		scale = vprev;
	else
	{
		time_slice = (ticks - transform->scales.time[prev]) / (transform->scales.time[next] - transform->scales.time[prev]);
		scale = float3::Lerp(vprev, vnext, (float)time_slice);
	}
}
