#include "ResourceBone.h"
#include "Application.h"
#include "ModuleResources.h"
#include "LoaderBone.h"

// ---------------------------------------------------------
ResourceBone::ResourceBone(UID uid) : Resource(uid, Resource::Type::bone)
{}

// ---------------------------------------------------------
ResourceBone::~ResourceBone()
{
	RELEASE_ARRAY(weigth_indices);
	RELEASE_ARRAY(weigths);
}

// ---------------------------------------------------------
bool ResourceBone::LoadInMemory()
{
	return App->resources->GetBoneLoader()->Load(this);
}

// ---------------------------------------------------------
void ResourceBone::Save(Config & config) const
{
	Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceBone::Load(const Config & config)
{
	Resource::Load(config);
}
