#ifndef __RESOURCE_ANIMATION_H__
#define __RESOURCE_ANIMATION_H__

#include "Resource.h"

class ResourceAnimation : public Resource
{
public:
	ResourceAnimation(UID id);
	virtual ~ResourceAnimation();

	bool LoadInMemory() override;

	void Save(Config& config) const override;
	void Load(const Config& config) override;

public:
	std::string name;
	double duration;
	double ticks_per_second;
};

#endif // __RESOURCE_ANIMATION_H__