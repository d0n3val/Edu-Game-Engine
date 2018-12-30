#ifndef __COMPONENT_WITH_RESOURCE_H__
#define __COMPONENT_WITH_RESOURCE_H__


// Interface for components that handle resources
#include "Globals.h"

class Config;
class Resource;


class ComponentWithResource
{
public:
	virtual ~ComponentWithResource();

	virtual bool SetResource(UID Resource) = 0;
	virtual const Resource* GetResource() const;
	UID GetResourceUID() const;

protected:

	void OnSaveResource(Config& config) const;
	void OnLoadResource(Config* config);

protected:
	UID resource = 0;
};

#endif // __COMPONENT_WITH_RESOURCE_H__
