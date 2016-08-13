#include "ComponentWithResource.h"
#include "Application.h"
#include "ModuleResources.h"

ComponentWithResource::~ComponentWithResource()
{
}

const Resource * ComponentWithResource::GetResource() const
{
	return App->resources->Get(resource);
}

UID ComponentWithResource::GetResourceUID() const
{
	return resource;
}
