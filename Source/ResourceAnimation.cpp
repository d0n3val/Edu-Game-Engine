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
	delete [] channels;
}

// ---------------------------------------------------------
bool ResourceAnimation::LoadInMemory()
{
}

// ---------------------------------------------------------
void ResourceAnimation::ReleaseFromMemory()
{
    // \todo:
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
bool ResourceAnimation::Import(const char* full_path, std::string& output)
{
}

