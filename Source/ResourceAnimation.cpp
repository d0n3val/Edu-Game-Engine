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
	config.AddString("Name", name.c_str());
	config.AddDouble("Duration", duration);
	config.AddDouble("Ticks", ticks_per_second);
}

// ---------------------------------------------------------
void ResourceAnimation::Load(const Config & config)
{
	Resource::Load(config);
	name = config.GetString("Name", "Unnamed");
	duration = config.GetDouble("Duration", 0.0);
	ticks_per_second = config.GetDouble("Ticks", 0.0);
}
