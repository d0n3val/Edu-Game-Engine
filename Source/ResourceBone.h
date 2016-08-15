#ifndef __RESOURCE_BONE_H__
#define __RESOURCE_BONE_H__

#include "Resource.h"
#include "Math.h"

class Resource;

class ResourceBone : public Resource
{
public:
	ResourceBone(UID id);
	virtual ~ResourceBone();

	bool LoadInMemory() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

public:
	
	uint num_weigths = 0;
	uint* weigth_indices = nullptr;
	float* weigths = nullptr;
	float4x4 offset;
	UID mesh = 0;
};

#endif // __RESOURCE_BONE_H__