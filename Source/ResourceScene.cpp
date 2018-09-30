#include "ResourceScene.h"
#include "Config.h"

// ---------------------------------------------------------
ResourceScene::ResourceScene(UID uid) : Resource(uid, Resource::Type::scene)
{}

// ---------------------------------------------------------
ResourceScene::~ResourceScene()
{
}

// ---------------------------------------------------------
bool ResourceScene::LoadInMemory()
{
	return false;
}

// ---------------------------------------------------------
void ResourceScene::ReleaseFromMemory()
{
}

// ---------------------------------------------------------
void ResourceScene::Save(Config & config) const
{
	Resource::Save(config);
}

// ---------------------------------------------------------
void ResourceScene::Load(const Config & config)
{
	Resource::Load(config);
}
