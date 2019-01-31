#ifndef __RESOURCE_ANIMATION_H__
#define __RESOURCE_ANIMATION_H__

#include "Resource.h"
#include "Math.h"

class ResourceAnimation : public Resource
{
public:

	ResourceAnimation(UID id);
	virtual ~ResourceAnimation();

	bool        LoadInMemory     () override;
    void        ReleaseFromMemory() override;

	void        Save             (Config& config) const override;
	void        Load             (const Config& config) override;

	static bool Import           (const char* full_path, unsigned first, unsigned last, std::string& output);

	float       GetDuration      () const { return duration; }

public:

	struct Channel
	{
        std::string    name;
        math::float3*  positions    = nullptr; 
        math::float4*  rotations    = nullptr;
		unsigned       count        = 0;
	};

    unsigned duration     = 0;
    unsigned num_channels = 0;
    Channel* channels     = nullptr;
};

#endif // __RESOURCE_ANIMATION_H__
