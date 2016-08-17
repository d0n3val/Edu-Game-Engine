#ifndef __RESOURCE_BONE_H__
#define __RESOURCE_BONE_H__

#include "Resource.h"
#include "Math.h"

class ResourceBone : public Resource
{
public:
	ALIGN_CLASS_TO_16

	ResourceBone(UID id);
	virtual ~ResourceBone();

	bool LoadInMemory() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

public:
	
	float4x4 offset;
	uint num_weigths = 0;
	uint* weigth_indices = nullptr;
	float* weigths = nullptr;
	UID uid_mesh = 0;
};

#endif // __RESOURCE_BONE_H__