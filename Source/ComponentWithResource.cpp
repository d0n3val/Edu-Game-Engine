#include "ComponentWithResource.h"
#include "Application.h"
#include "ModuleResources.h"
#include "Config.h"

#include "Leaks.h"

ComponentWithResource::~ComponentWithResource()
{
    App->resources->Get(resource)->Release();
}

const Resource * ComponentWithResource::GetResource() const
{
	return App->resources->Get(resource);
}

UID ComponentWithResource::GetResourceUID() const
{
	return resource;
}

void ComponentWithResource::OnSaveResource(Config & config) const
{
	config.AddUID("Resource", resource);
}

void ComponentWithResource::OnLoadResource(Config * config)
{
	SetResource(config->GetUID("Resource", 0));
}
