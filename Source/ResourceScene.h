#ifndef __RESOURCE_SCENE_H__
#define __RESOURCE_SCENE_H__

#include "Resource.h"

class ResourceScene : public Resource
{
	friend class ModuleSceneLoader;

public:

public:
	ResourceScene(UID id);
	virtual ~ResourceScene();

	bool LoadInMemory() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

public:
};

#endif // __RESOURCE_SCENE_H__