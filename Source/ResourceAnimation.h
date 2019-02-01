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

    bool        Save             (std::string& output) const;
	static bool Import           (const char* full_path, unsigned first, unsigned last, std::string& output);

	unsigned    GetDuration      () const { return duration; }

public:

	struct Channel
	{
        std::string name;
        float3*     positions     = nullptr; 
        Quat*       rotations     = nullptr;
		uint        num_positions = 0;
		uint        num_rotations = 0;
	};

    uint     duration     = 0;
    uint     num_channels = 0;
    Channel* channels     = nullptr;
};

#endif // __RESOURCE_ANIMATION_H__
