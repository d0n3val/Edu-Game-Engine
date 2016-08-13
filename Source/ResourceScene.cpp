#include "ResourceScene.h"
#include "Config.h"

// ---------------------------------------------------------
ResourceScene::ResourceScene(UID uid) : Resource(uid, Resource::Type::audio)
{}

// ---------------------------------------------------------
ResourceScene::~ResourceScene()
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
