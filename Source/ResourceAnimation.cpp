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
